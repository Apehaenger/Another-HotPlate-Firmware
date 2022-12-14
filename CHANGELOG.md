# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!--
## [Unreleased]

### Added 

### Removed

- tbd
- something from [@someone](https://github.com/someone).

-->

## [0.5.0] - 2022-11-27

### Added 

- Added [PID Tuner](https://pidtuner.com) output
- Added average temperatures for more stable temperature display/handling 
- Added "Max. Temp" setting (which is required for PID Tuner steps)
- Added Changelog

### Changed

- Enlarged PID constant length from 3 to 4 int digits
- Improved some internal var names
- Optimized Thermocouple class and implemented average temperature
- Moved profile handling into a separate class
- Resturctured handling of objects via globals (instead of space wasting pointer/reference parameter)
- Changed initial PID-Tuner wait time and saved some resources

### Fixed

- Fixed possibliy wrong PID derivate calculation as discussed [here](https://github.com/r-downing/AutoPID/issues/4)


## [0.4.0] - 2022-11-08

### Added

- "Manual" or (open-end) "Reflow-Profile" Mode
- Reflow profiles for (low temp.) "Sn42/Bi57.6/Ag0.4" as well as (high temp.) "Sn96.5/Ag3.0/Cu0"

### Changed

- Moved most code into separate classes for higher flexibility and safer var/pointers
- Stripped down fonts, to get some more flash space

## 0.3.0 - 2022-10-26

### Added

- Built-in Setup
- Adjustabled PID constants
- BangON and BangOFF values
- Config storage into EEPROM

[unreleased]: https://github.com/Apehaenger/Another-HotPlate-Firmware/compare/v0.5.0...develop
[0.5.0]: https://github.com/Apehaenger/Another-HotPlate-Firmware/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/Apehaenger/Another-HotPlate-Firmware/releases/v0.4.0
