# Coding standard

This is the coding standard for Group 12's project (NF-01). It is based on the style of the course template (`project/project.ino` from the BTH/LilyGO skeleton), so new code looks the same as the code we started from. Reviewers check every pull request against these rules.

## Naming

| What | Style | Example |
|---|---|---|
| Functions | `snake_case`, starting with a verb | `show_departures()`, `connect_wifi()` |
| Event callbacks (LVGL) | `on_<thing>_<event>` | `on_stop_changed()`, `on_show_clicked()` |
| Variables | `snake_case` | `current_stop`, `departure_list` |
| Constants and fixed lists | `UPPER_CASE` | `STOPS`, `STOP_IDS`, `FW_VERSION` |
| Structs | `PascalCase` | `Departure` |
| Struct fields | `snake_case` | `delay_min` |
| Tiles and their title labels | `t1`, `t2`, `t3` and `t1_label` ... (as in the template) | `t3_label` |

Use names that say what something is. Avoid one-letter names, except loop counters (`i`) and the LVGL event parameter (`e`).

## Formatting

- Indent with 2 spaces, no tabs.
- A function's opening brace goes on its own line. For `if`, `for` and `while`, the brace goes on the same line.
- Short one-line `if` statements without braces are allowed when they fit on one line.
- Keep lines readable. Break long lines rather than scrolling far to the right.

```cpp
// Function: Example of the layout
static void example(int value)
{
  if (value > 0) {
    Serial.println(value);
  }
}
```

## Comments

- Every function has a comment directly above it that starts with `// Function:` and says what it does in one line.
- `setup()` and `loop()` keep the template's `// Must have function:` comments.
- Write the requirement ID in the comment when code belongs to a requirement, for example `(FR-02)`.
- Comment *why* when something is not obvious, for example `// skip certificate check for now`.
- Comments are in English.

## Structure

- All our code is in `project/`. The skeleton files (`src/`, `boards/`, `platformio.ini` and the other template files) are not changed (NF-04).
- Order in `project.ino`: includes, then constants and global variables, then helper functions, then screen functions, then `setup()` and `loop()`.
- Keep functions short and focused on one job. Reuse the helpers (`add_label()`, `add_button()`, `make_column()`) instead of copying code.
- Global variables are `static`, as in the template.
- Text shown on the screen is ASCII only (the fonts have no å, ä or ö), so `Grasvik` instead of `Gräsvik`.

## Secrets

- Wi-Fi details and the Trafiklab API key go only in `project/secrets.h`, which is in `.gitignore` and is never committed.
- `project/secrets.example.h` shows which values are needed and is kept up to date.

## Git and pull requests

- Never commit directly to `main`. Every change goes through a branch and a pull request.
- Branch names: lowercase with dashes, starting with the requirement ID, for example `fr-02-live-data` or `nf-01-coding-standard`.
- Commit messages: `<requirement ID>: <what the commit does>`, for example `FR-02: Trafiklab test request (serial output only)`.
- The pull request description links the issue: `Closes #N` when the PR finishes the requirement, `Part of #N` when it only does part of it.
- Every pull request needs one approving review from another group member. The reviewer checks the code against this standard and tests it on the board when the change affects the device.
