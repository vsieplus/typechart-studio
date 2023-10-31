
## v1.0.0 - 10/31/2023

### Enhancements
- Refactor / cleanup overall codebase
- Refine builds to be more portable

### Bug Fixes
- Fixed a segmentation fault when shutting down the audio system

## v0.3.0 - 4/26/2023

### Enhancements

- Update default font
- Start window maximized
- Change edit window titles to avoid conflicts
- Update default window sizes
- Update help tooltips, dialog boxes
- Allow inserting spacebar with a single click
- Allow text input for default directory preferences

### Bug Fixes

- Fix implementation of `removeSkip()`

## v0.2.5 - 2/26/2023

### Enhancements
- Implement `operator+/-` for `Beatpos`
- Implement `operator==` for `NoteSequenceItem`
- Update window icon
- Refine shift-click behavior
- Add genre to `SongInfo`
- Implement custom json serialization for saving song/chart files
- Allow editing song/chart info from the edit window
- Update minimum timeline zoom to 0.25
- Added `ShiftNote` action

### Bug Fixes
- Fix windows release builds
- Fix `fs::path` usage for compatibility with mingw
- Fix beatpos calculation for `insertItems`
- Fix section data handling when multiple edit windows open
- Correctly clamp music preview start/stop values
- Refine save/save as functionality

## v0.2.3 - 12/1/2022

### Enhancements
- Add 'difficulty' option field, separate from level, during level creation
- Remove help menu
- General style improvements

### Bug Fixes
- Song position floating point precision changed to double, which was causing rounding errors in `setSongBeatPosition()`
- Correctly update key frequency tables when flipping notes
- Fix skip resetting when setting song position

## v0.2.2 - 7/24/2022

### Enhancements
- Desubmodule imgui libs
- Add support for skip rendering
- Add support for bpm change interpolation (currently w/o rendering)
- Remove help menu
- Add undo/redo for note flipping
- Save Preferences as `.json` instead of `.bin`
- Add option to disable notesounds
- Prettyprint json
- Darker bg color


### Bug Fixes
- Audio skipping
- Misaligned highlight for timeline selection
- Add winpthread to w64 build
- Fix left click persistence when adding skips / bottom notes
- Fix beatsplit rounding
- Fix over/under scroll before or after full length of song

## v0.2.0 - 5/19/2022

### Enhancements
- Fix most recent files menu
- implement insertion, editing, deletion of skips and stops
- Implement note selection, copy, cut, paste
- Implement undo, redo
  - note and item placement, deletion, editing
- Add note 'flipping' based on keyboard layout
- Generalize Audio system to handle multiple music files, allow multiple opened edit windows
- Track 'unsaved' status correctly

### Bug Fixes
- Note selection display not adapting to zoom, beat pos
- Random crashes when accessing "Recent"

## v0.1.2 - 5/14/2022

### Enhancements
- support for stops and skips insertion, saving, loading
- scale scroll by current zoom

### Bug Fixes
- opening chart files, closing edit windows no longer crashes
- fix most recent file display
- fix dragged note display
- save music preview position

## v0.1.1 - 5/11/2022

### Enhancements
- note frequency stats + visualization 
- add function keys using keyboard

### Bug fixes
- hold note rendering
- loading function key hold notes
- correct lane assignments
- audio delay
- shared_ptr < operator

## v0.1.0 - 5/7/2022

- Initial release of Typechart Studio
