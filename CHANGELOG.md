Kasia Framework: change log
=======================

v0.1.0 (2023-02-21) Stable BETA
-------

* WiFi connection credentials are persisted
* Ability to bindData
* Logging both to Serial and to Web

v0.1.1 (2023-02-23) WiFi Overwatch
-------

* Non-Kasia headers were moved to src to avoid confusion
* Fixed issue with WaitUntilConnected logic
* Added WiFi Overwatch to show WiFi connection errors if not successfully connected at the startup, while still ensuring a non-blocking start 
