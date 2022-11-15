# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added 

- Added [PID Tuner](https://pidtuner.com) output
- Added average temperatures for more stable temperature display/handling 
- Added Changelog

### Changed

- Improved some internal var names
- Implemeted "Runnable" super-class
- Optimized Thermocouple class and implemented average temperature
- Changed from "pass by pointer" to "pass by reference"
- Moved profile handling into a separate class
- Resturctured handling of objects via globals (instead of space wasting pointer/reference parameter)
- General handling of 
- Changed initial PID-Tuner wait time and saved some resources
- Removed Runnable super-class and save approx. 
- Cleaned up Hotplate & Ui cpp/h files

### Fixed

- Fixed possibliy wrong PID derivate calculation as discussed [here](https://github.com/r-downing/AutoPID/issues/4)

<!-- Further samples:


### Removed

- tbd
- something from [@someone](https://github.com/someone).

-->

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

[unreleased]: https://github.com/Apehaenger/Another-HotPlate-Firmware/compare/v0.4.0...develop
<!--
[0.4.x]: https://github.com/Apehaenger/Another-HotPlate-Firmware/compare/v0.4.0...v0.4.x -->
[0.4.0]: https://github.com/Apehaenger/Another-HotPlate-Firmware/releases/v0.4.0
