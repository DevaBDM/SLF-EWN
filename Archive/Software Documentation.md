The app now starts on live serial, not simulation.  
On launch it scans USB-UART ports, prefers CH340 / CP210 / FTDI-style receivers, opens the port at 115200 8N1, and streams frames into the dashboard and SQLite as they arrive. Simulation is off by default and only runs if an admin checks it in Settings → LINK.  
That checkbox is an explicit demo mode: turning it on pauses the live port so fake packets cannot mix with real ones. Unchecking it reconnects the receiver.  
Usage:  
Plug in the LoRa USB receiver.  
Sign in (admin to change ports; guests still see live data).  
The top bar should show the port name (for example `COM5`), not `SIMULATION`.  
If it does not auto-open, pick the port in the sidebar or Settings → LINK and click Connect.  
Accepted on the wire:  
Binary 12-byte frames: `ID | moisture | tiltX/Y/Z | status | CRC-8`  
ASCII CSV lines: `A104,31.4,0.7,-0.5,1.0`  
Settings → LINK shows bytes in, frames decoded, and dropped bytes, which distinguishes a silent cable from a framing mismatch. The Qt Creator project is synced with this build.
