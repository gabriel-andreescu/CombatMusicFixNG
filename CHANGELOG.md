# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

## [2.0.0] - 2026-09-20

### Added

- Add a setting for debug logging
- Add a setting to run cleanup only after loading a save

### Changed

- **Breaking change:** Move user settings to `MCM/Settings/CombatMusicFixNG.ini`

### Fixed

- Support Skyrim Steam AE 1.7.104
- Stop lingering music without requiring an object selected in the console
- Execute music commands on the game thread
- Leave combat music playing if combat starts during the cleanup delay

## [1.2.0] - 2025-06-07

### Added

- Make the music types to stop configurable through an INI file

## [1.1.0] - 2024-01-30

### Added

- Support SE, AE, and VR through CommonLibSSE-NG

### Fixed

- Stop additional combat music types
