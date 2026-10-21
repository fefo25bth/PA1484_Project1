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
