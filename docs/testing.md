# Test log

Manual tests on the device, recorded for every requirement (NF-05). Each requirement is tested against its acceptance criteria before it is moved to Done. Before every sprint demo, all earlier tests are run again to check that nothing has broken.

| Date | Version | Requirement | What was tested | Result | Tester |
|---|---|---|---|---|---|
| 2026-10-20 | v0.4.0 | FR-01 | Start screen shows project name, version, group number and all members after reset, without scrolling | Pass | Anton |
| 2026-10-20 | v0.4.0 | FR-03 | Swiping left/right moves between start, settings and departure screens; tapping does not change screen | Pass | Anton |
| 2026-10-20 | v0.4.0 | FR-04 | Settings screen is reached by swiping from the departure screen | Pass | Anton |
| 2026-10-20 | v0.4.0 | FR-05 | Stop dropdown lists all 5 stops; chosen stop is shown on the departure screen (demo data) | Pass | Anton |
| 2026-10-20 | v0.4.0 | FR-06 | Transport type dropdown (All/Bus/Train/Ferry) hides other types (demo data) | Pass | Anton |
| 2026-10-21 | v0.4.1 | FR-02 | Trafiklab request for all 5 stops returns HTTP 200 and lists departures in the serial monitor | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-02 | Departure screen shows time, line, destination and status for the chosen stop (live data) | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-02 | Times match Trafiklab for the same stop within 1 minute | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-02 | A delay of 60 s or more shows "+N min" (yellow); a cancelled departure shows "Cancelled" (red) | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-02 | Without Wi-Fi the screen shows "No live data. Check the Wi-Fi connection." | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-05 | Changing the stop in settings shows live departures for that stop | Pass | Felix |
| 2026-11-18 | v0.5.0 | FR-06 | Transport type filter (All/Bus/Train/Ferry) works on live data | Pass | Felix |
| 2026-11-18 | v0.5.0 | NF-03 | "Data: Trafiklab.se (CC BY 4.0)" is visible on the departure screen | Pass | Felix |
| 2026-11-18 | v0.6.0 | FR-07 | After "Save as default" and a restart, the saved stop and type are selected | Pass | Felix |
| 2026-11-18 | v0.6.0 | FR-08 | After changing stop/type and pressing "Reset to default", the saved defaults are selected again | Pass | Felix |
| 2026-11-18 | v0.6.0 | FR-01, FR-03, FR-04 | Regression: start screen, swiping and settings screen still work | Pass | Felix |
