# Change Log

All notable changes to this project will be documented in this file. This project adheres to [Semantic Versioning](https://semver.org).

## v0.0.2 - 2020-04-20

### Added

- Add thread number control parameter when initialize `PasswordMaker`
- Add license notice for third-part libraries
- Add command line parameters `-c/--config` and `-t/--thread`

### Fixed

- Fix the timing of serialization (#1)

### Changed

- Use `cbegin`/`cend` to replace `begin`/`end` in const variables
- Remove unusable `_appedLock`
- Update README
- Add `size()` function to get worker number in `ThreadPool`

## v0.0.1 - 2020-04-19

- Initial repo.