# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## WIP 306 - 2020-02-28
### Added
- Header file generation to the 3DS Max Orbiter mesh tool.
- Landing, taxi, and docking light.
- ADI ball.
- 2 Hydraulic systems.
- Gear emergency release system.
- Fuel lines.
- Undocking handle.
- HUD color and brightness buttons.
- Anti-slip autopilot.
- Fuel dump system.
- Bay door switch.
- Docking supply system.
- Warning system, with V1 and rotate callouts.
- RCS doors logic.
- APU Pack B.
### Changed
- 3DS Max tools are now utilities instead of macro scripts.
- RCS and OMS are now seperated systems.
- Ramcaster engine coefficient.
- Landing gears damping coefficient.
### Removed
- Useless and inoperative switches.

## WIP 305 - 2020-02-06
### Added
- UCSO support.
- Missing cockpit sounds.
- Parking brake with handle in the cockpit.
- Ramcaster engine handle is clickable in the cockpit.
- Autopilot and HUD mode buttons.
- RCS and ACS knobs.
- APU EICAS screen.
- Docking RCS mode.
- RCS and OMS engines fuel pumps.
- APU must be on to engage ACS.
### Fixed
- Touchdown points.
- Vessel size is set 1 meter.
- Scenario load bug.
- Nosewheel steering.
- Wheels braking.
- HUD data.
- Landing gear sound not played when extending the gear.
- APU cycle switch out of place.
- HUD is too dim sometimes.
- Wing position indicator on ACS EICAS screen reversing the actual wing position.
### Changed
- Source code cleanup. No more compiler warnings.
- Upgraded Orbiter Sound SDK to 4.0.
- EICAS engine displays are now for both engines. Before, both engine displays are only showing the left engine values.
- Textures folder is now G422 instead of DGa2.
- Vertical lift coefficient.
- Upgraded to WIP 304 mesh (new EICAS screens).
- The checklist to match the current version.
- Ramcaster engine coefficient.
- Revised EICAS displays.
### Removed
- Useless files, libraries and textures.
- APU Pack B.