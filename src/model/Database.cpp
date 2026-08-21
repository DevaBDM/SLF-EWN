#include "Database.h"

#include <QCryptographicHash>
#include <QDir>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariantMap>

#include <algorithm>

namespace Slf {

namespace {

constexpr int kPbkdf2Iterations = 60000;
constexpr int kKeyLength = 32;
constexpr int kFlushIntervalMs = 4000;
constexpr int kMaxPendingBeforeFlush = 200;
const char *kDefaultAdminPassword = "admin";
const char *kConnectionName = "slf-ewn-local";

int roleToInt(Role r) { return static_cast<int>(r); }

Role roleFromInt(int v)
{
    switch (v) {
    case 2: return Role::Admin;
    case 1: return Role::Guest;
    default: return Role::None;
    }
}

QDateTime fromMsecs(qint64 ms)
{
    return ms > 0 ? QDateTime::fromMSecsSinceEpoch(ms) : QDateTime();
}

} // namespace

Database::Database(QObject *parent)
    : QObject(parent)
{
    flushTimer_.setInterval(kFlushIntervalMs);
    connect(&flushTimer_, &QTimer::timeout, this, &Database::flush);
}

Database::~Database()
{
    close();
}

bool Database::open(QString *errorOut)
{
    if (db_.isOpen())
        return true;

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) {
        lastError_ = QStringLiteral("No writable application data location");
        if (errorOut)
            *errorOut = lastError_;
        return false;
    }
    QDir().mkpath(dir);
    path_ = dir + QStringLiteral("/slfewn.db");

    db_ = QSqlDatabase::contains(QLatin1String(kConnectionName))
              ? QSqlDatabase::database(QLatin1String(kConnectionName))
              : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QLatin1String(kConnectionName));
    db_.setDatabaseName(path_);

    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        if (errorOut)
            *errorOut = lastError_;
        return false;
    }

    QSqlQuery pragma(db_);
    pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    pragma.exec(QStringLiteral("PRAGMA synchronous = NORMAL"));
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    if (!createSchema(errorOut))
        return false;

    seedDefaultAccounts();

    QSqlQuery count(db_);
    if (count.exec(QStringLiteral("SELECT COUNT(*) FROM samples")) && count.next())
        sampleCount_ = count.value(0).toLongLong();

    flushTimer_.start();
    return true;
}

void Database::close()
{
    if (!db_.isOpen())
        return;
    flush();
    flushTimer_.stop();
    // Fold the write-ahead log back into the main file. Without this the .db on
    // disk stays a 4 KB stub while the real data sits in slfewn.db-wal, which
    // makes a working store look empty to anyone inspecting the folder.
    QSqlQuery checkpoint(db_);
    checkpoint.exec(QStringLiteral("PRAGMA wal_checkpoint(TRUNCATE)"));
    db_.close();
    db_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(QLatin1String(kConnectionName));
}

bool Database::createSchema(QString *errorOut)
{
    const QStringList statements = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS users ("
                       "username TEXT PRIMARY KEY COLLATE NOCASE,"
                       "role INTEGER NOT NULL,"
                       "salt TEXT NOT NULL,"
                       "hash TEXT NOT NULL,"
                       "created_at INTEGER NOT NULL,"
                       "last_login INTEGER)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS settings ("
                       "key TEXT PRIMARY KEY,"
                       "value TEXT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS nodes ("
                       "device_id INTEGER PRIMARY KEY,"
                       "zone_id TEXT,"
                       "zone_label TEXT,"
                       "map_x REAL,"
                       "map_y REAL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS samples ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "device_id INTEGER NOT NULL,"
                       "ts INTEGER NOT NULL,"
                       "moisture REAL,"
                       "tilt_x REAL,"
                       "tilt_y REAL,"
                       "tilt_z REAL,"
                       "tilt_mag REAL,"
                       "status INTEGER)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_samples_device_ts ON samples(device_id, ts)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS alerts ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "ts INTEGER NOT NULL,"
                       "device_id INTEGER NOT NULL,"
                       "zone TEXT,"
                       "event_type INTEGER NOT NULL,"
                       "trigger_value TEXT,"
                       "acknowledged INTEGER NOT NULL DEFAULT 0,"
                       "acknowledged_by TEXT,"
                       "acknowledged_at INTEGER)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_alerts_ts ON alerts(ts)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS audit_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "ts INTEGER NOT NULL,"
                       "username TEXT,"
                       "action TEXT NOT NULL,"
                       "detail TEXT)"),
    };

    for (const QString &sql : statements) {
        QSqlQuery q(db_);
        if (!q.exec(sql)) {
            lastError_ = q.lastError().text();
            if (errorOut)
                *errorOut = lastError_;
            return false;
        }
    }
    return true;
}

void Database::seedDefaultAccounts()
{
    if (!userExists(QStringLiteral("admin")))
        createUser(QStringLiteral("admin"), QLatin1String(kDefaultAdminPassword), Role::Admin, nullptr);
    if (!userExists(QStringLiteral("guest")))
        createUser(QStringLiteral("guest"), QStringLiteral("guest"), Role::Guest, nullptr);
}

QString Database::makeSalt()
{
    QByteArray salt(16, '\0');
    auto *gen = QRandomGenerator::system();
    for (int i = 0; i < salt.size(); ++i)
        salt[i] = static_cast<char>(gen->bounded(256));
    return QString::fromLatin1(salt.toHex());
}

QString Database::hashPassword(const QString &password, const QString &salt)
{
    const QByteArray derived = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        QByteArray::fromHex(salt.toLatin1()),
        kPbkdf2Iterations,
        kKeyLength);
    return QString::fromLatin1(derived.toHex());
}

bool Database::authenticate(const QString &username, const QString &password, Role *roleOut, QString *errorOut)
{
    if (!db_.isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Local store unavailable");
        return false;
    }

    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT role, salt, hash FROM users WHERE username = ?"));
    q.addBindValue(username.trimmed());
    if (!q.exec() || !q.next()) {
        if (errorOut)
            *errorOut = QStringLiteral("Unknown operator or wrong password");
        return false;
    }

    const int role = q.value(0).toInt();
    const QString salt = q.value(1).toString();
    const QString stored = q.value(2).toString();

    if (hashPassword(password, salt) != stored) {
        if (errorOut)
            *errorOut = QStringLiteral("Unknown operator or wrong password");
        return false;
    }

    if (roleOut)
        *roleOut = roleFromInt(role);
    return true;
}

bool Database::createUser(const QString &username, const QString &password, Role role, QString *errorOut)
{
    const QString name = username.trimmed();
    if (name.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("Username required");
        return false;
    }
    if (userExists(name)) {
        if (errorOut)
            *errorOut = QStringLiteral("That operator already exists");
        return false;
    }

    const QString salt = makeSalt();
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO users (username, role, salt, hash, created_at)"
                             " VALUES (?, ?, ?, ?, ?)"));
    q.addBindValue(name);
    q.addBindValue(roleToInt(role));
    q.addBindValue(salt);
    q.addBindValue(hashPassword(password, salt));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        if (errorOut)
            *errorOut = lastError_;
        return false;
    }
    return true;
}

bool Database::changePassword(const QString &username, const QString &newPassword, QString *errorOut)
{
    if (newPassword.size() < 4) {
        if (errorOut)
            *errorOut = QStringLiteral("Password must be at least 4 characters");
        return false;
    }

    const QString salt = makeSalt();
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE users SET salt = ?, hash = ? WHERE username = ?"));
    q.addBindValue(salt);
    q.addBindValue(hashPassword(newPassword, salt));
    q.addBindValue(username.trimmed());
    if (!q.exec() || q.numRowsAffected() <= 0) {
        lastError_ = q.lastError().text();
        if (errorOut)
            *errorOut = lastError_.isEmpty() ? QStringLiteral("No such operator") : lastError_;
        return false;
    }
    return true;
}

bool Database::deleteUser(const QString &username, QString *errorOut)
{
    const QString name = username.trimmed();
    if (name.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0) {
        if (errorOut)
            *errorOut = QStringLiteral("The primary admin account cannot be removed");
        return false;
    }
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM users WHERE username = ?"));
    q.addBindValue(name);
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        if (errorOut)
            *errorOut = lastError_;
        return false;
    }
    return true;
}

bool Database::userExists(const QString &username)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT 1 FROM users WHERE username = ?"));
    q.addBindValue(username.trimmed());
    return q.exec() && q.next();
}

bool Database::usesDefaultPassword(const QString &username)
{
    Role r = Role::None;
    if (username.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0)
        return authenticate(username, QLatin1String(kDefaultAdminPassword), &r, nullptr);
    return false;
}

QVector<UserRecord> Database::users()
{
    QVector<UserRecord> out;
    QSqlQuery q(db_);
    if (!q.exec(QStringLiteral("SELECT username, role, created_at, last_login FROM users"
                               " ORDER BY role DESC, username ASC")))
        return out;
    while (q.next()) {
        UserRecord u;
        u.username = q.value(0).toString();
        u.role = roleFromInt(q.value(1).toInt());
        u.createdAt = fromMsecs(q.value(2).toLongLong());
        u.lastLogin = fromMsecs(q.value(3).toLongLong());
        u.defaultPassword = usesDefaultPassword(u.username);
        out.append(u);
    }
    return out;
}

void Database::touchLogin(const QString &username)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE users SET last_login = ? WHERE username = ?"));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(username.trimmed());
    q.exec();
}

QString Database::setting(const QString &key, const QString &fallback)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT value FROM settings WHERE key = ?"));
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return fallback;
}

void Database::setSetting(const QString &key, const QString &value)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO settings (key, value) VALUES (?, ?)"
                             " ON CONFLICT(key) DO UPDATE SET value = excluded.value"));
    q.addBindValue(key);
    q.addBindValue(value);
    q.exec();
}

QVector<NodeProfile> Database::loadProfiles()
{
    QVector<NodeProfile> out;
    QSqlQuery q(db_);
    if (!q.exec(QStringLiteral("SELECT device_id, zone_id, zone_label, map_x, map_y FROM nodes")))
        return out;
    while (q.next()) {
        NodeProfile p;
        p.deviceId = static_cast<quint16>(q.value(0).toUInt());
        p.zoneId = q.value(1).toString();
        p.zoneLabel = q.value(2).toString();
        p.mapPos = QPointF(q.value(3).toDouble(), q.value(4).toDouble());
        out.append(p);
    }
    return out;
}

void Database::saveProfile(const NodeProfile &profile)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO nodes (device_id, zone_id, zone_label, map_x, map_y)"
                             " VALUES (?, ?, ?, ?, ?)"
                             " ON CONFLICT(device_id) DO UPDATE SET"
                             " zone_id = excluded.zone_id,"
                             " zone_label = excluded.zone_label,"
                             " map_x = excluded.map_x,"
                             " map_y = excluded.map_y"));
    q.addBindValue(profile.deviceId);
    q.addBindValue(profile.zoneId);
    q.addBindValue(profile.zoneLabel);
    q.addBindValue(profile.mapPos.x());
    q.addBindValue(profile.mapPos.y());
    q.exec();
}

void Database::queueSample(quint16 deviceId, const Sample &sample, NodeStatus status)
{
    if (!db_.isOpen())
        return;
    PendingSample p;
    p.deviceId = deviceId;
    p.sample = sample;
    p.status = status;
    pending_.append(p);
    if (pending_.size() >= kMaxPendingBeforeFlush)
        flush();
}

void Database::flush()
{
    if (pending_.isEmpty() || !db_.isOpen())
        return;

    const QVector<PendingSample> batch = pending_;
    pending_.clear();

    db_.transaction();
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO samples"
                             " (device_id, ts, moisture, tilt_x, tilt_y, tilt_z, tilt_mag, status)"
                             " VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const PendingSample &p : batch) {
        q.addBindValue(p.deviceId);
        q.addBindValue(p.sample.timestamp.toMSecsSinceEpoch());
        q.addBindValue(p.sample.moisture);
        q.addBindValue(p.sample.tiltXDeg);
        q.addBindValue(p.sample.tiltYDeg);
        q.addBindValue(p.sample.tiltZDeg);
        q.addBindValue(p.sample.tiltMagDeg);
        q.addBindValue(static_cast<int>(p.status));
        if (q.exec()) {
            ++sampleCount_;
            ++rowsWritten_;
        } else {
            lastError_ = q.lastError().text();
        }
    }
    db_.commit();
    lastWrite_ = QDateTime::currentDateTime();
}

void Database::insertSamples(quint16 deviceId, const QVector<Sample> &samples)
{
    if (samples.isEmpty() || !db_.isOpen())
        return;

    db_.transaction();
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO samples"
                             " (device_id, ts, moisture, tilt_x, tilt_y, tilt_z, tilt_mag, status)"
                             " VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const Sample &s : samples) {
        q.addBindValue(deviceId);
        q.addBindValue(s.timestamp.toMSecsSinceEpoch());
        q.addBindValue(s.moisture);
        q.addBindValue(s.tiltXDeg);
        q.addBindValue(s.tiltYDeg);
        q.addBindValue(s.tiltZDeg);
        q.addBindValue(s.tiltMagDeg);
        q.addBindValue(static_cast<int>(NodeStatus::Unknown));
        if (q.exec()) {
            ++sampleCount_;
            ++rowsWritten_;
        } else {
            lastError_ = q.lastError().text();
        }
    }
    db_.commit();
    lastWrite_ = QDateTime::currentDateTime();
}

QVector<Sample> Database::loadSamples(quint16 deviceId, qint64 sinceMsecs, int limit)
{
    QVector<Sample> out;
    if (!db_.isOpen())
        return out;

    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT ts, moisture, tilt_x, tilt_y, tilt_z, tilt_mag FROM samples"
                             " WHERE device_id = ? AND ts >= ?"
                             " ORDER BY ts DESC LIMIT ?"));
    q.addBindValue(deviceId);
    q.addBindValue(sinceMsecs);
    q.addBindValue(limit);
    if (!q.exec())
        return out;

    while (q.next()) {
        Sample s;
        s.timestamp = QDateTime::fromMSecsSinceEpoch(q.value(0).toLongLong());
        s.moisture = q.value(1).toFloat();
        s.tiltXDeg = q.value(2).toFloat();
        s.tiltYDeg = q.value(3).toFloat();
        s.tiltZDeg = q.value(4).toFloat();
        s.tiltMagDeg = q.value(5).toFloat();
        out.append(s);
    }
    std::reverse(out.begin(), out.end());
    return out;
}

bool Database::hasSamples()
{
    QSqlQuery q(db_);
    return q.exec(QStringLiteral("SELECT 1 FROM samples LIMIT 1")) && q.next();
}

qint64 Database::sampleCount()
{
    return sampleCount_;
}

void Database::pruneOlderThan(int days)
{
    if (!db_.isOpen())
        return;
    const qint64 cutoff = QDateTime::currentDateTime().addDays(-days).toMSecsSinceEpoch();
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM samples WHERE ts < ?"));
    q.addBindValue(cutoff);
    if (q.exec())
        sampleCount_ = qMax<qint64>(0, sampleCount_ - q.numRowsAffected());
}

qint64 Database::insertAlert(const AlertRecord &alert)
{
    if (!db_.isOpen())
        return -1;

    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO alerts (ts, device_id, zone, event_type, trigger_value, acknowledged)"
                             " VALUES (?, ?, ?, ?, ?, 0)"));
    q.addBindValue(alert.timestamp.toMSecsSinceEpoch());
    q.addBindValue(alert.deviceId);
    q.addBindValue(alert.zone);
    q.addBindValue(static_cast<int>(alert.type));
    q.addBindValue(alert.triggerValue);
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

QVector<AlertRecord> Database::loadAlerts(int limit)
{
    QVector<AlertRecord> out;
    if (!db_.isOpen())
        return out;

    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT id, ts, device_id, zone, event_type, trigger_value, acknowledged"
                             " FROM alerts ORDER BY ts DESC, id DESC LIMIT ?"));
    q.addBindValue(limit);
    if (!q.exec())
        return out;

    while (q.next()) {
        AlertRecord a;
        a.id = q.value(0).toLongLong();
        a.timestamp = QDateTime::fromMSecsSinceEpoch(q.value(1).toLongLong());
        a.deviceId = static_cast<quint16>(q.value(2).toUInt());
        a.zone = q.value(3).toString();
        a.type = static_cast<EventType>(q.value(4).toInt());
        a.triggerValue = q.value(5).toString();
        a.acknowledged = q.value(6).toInt() != 0;
        out.append(a);
    }
    return out;
}

void Database::setAlertAcknowledged(qint64 alertId, const QString &by)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE alerts SET acknowledged = 1, acknowledged_by = ?, acknowledged_at = ?"
                             " WHERE id = ?"));
    q.addBindValue(by);
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(alertId);
    q.exec();
}

void Database::acknowledgeDeviceAlerts(quint16 deviceId, const QString &by)
{
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE alerts SET acknowledged = 1, acknowledged_by = ?, acknowledged_at = ?"
                             " WHERE device_id = ? AND acknowledged = 0"));
    q.addBindValue(by);
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(deviceId);
    q.exec();
}

void Database::clearAlerts()
{
    QSqlQuery q(db_);
    q.exec(QStringLiteral("DELETE FROM alerts"));
}

void Database::audit(const QString &username, const QString &action, const QString &detail)
{
    if (!db_.isOpen())
        return;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO audit_log (ts, username, action, detail) VALUES (?, ?, ?, ?)"));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(username);
    q.addBindValue(action);
    q.addBindValue(detail);
    q.exec();
}

QVector<QVariantMap> Database::recentAudit(int limit)
{
    QVector<QVariantMap> out;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT ts, username, action, detail FROM audit_log"
                             " ORDER BY ts DESC LIMIT ?"));
    q.addBindValue(limit);
    if (!q.exec())
        return out;
    while (q.next()) {
        QVariantMap m;
        m[QStringLiteral("timestamp")] =
            QDateTime::fromMSecsSinceEpoch(q.value(0).toLongLong())
                .toString(QStringLiteral("yyyy-MM-dd  HH:mm:ss"));
        m[QStringLiteral("username")] = q.value(1).toString();
        m[QStringLiteral("action")] = q.value(2).toString();
        m[QStringLiteral("detail")] = q.value(3).toString();
        out.append(m);
    }
    return out;
}

} // namespace Slf
