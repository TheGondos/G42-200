# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## WIP 307 - 2021-03-04
### Added
- New scenarios and images by Matias Saibene.
- Burner engage knob.
- Main, ASF, and oxidizer tanks full, gear up, gear up and locked, gear down, and gear down and locked announcements.
- Gear is up warning if below 300 meters and descending.
- APU offline warning if tried to manipulate a moving part and the APU is off.
- Antislip AP on and off sounds.
- Docking port can't be used if the payload bay doors are closed.
- Engines contrail.
- Gear indicator on the HUD.
- Center of gravity shifting.
- ApT and PeT on the orbit HUD.
- Shift + S shortcut to start or stop the engines.
- Percent sign in the APU quantity to make it more clear.
- UCSO cargoes can now be packed and unpacked.
### Changed
- Dynamic XRSound is used instead of Orbiter Sound SDK.
- Hydraulic pressure is now required to operate the wheel brakes.
- Moving parts "saving" logic.
- Renamed landing gears to landing gear.
- Sound implementation for moving parts.
- Repository structure.
- UCSO operation way. Cargo mode have to be activated (Shift + C) which will change the HUD display.
- User manual.
- Engine positions are now correct on X and Z axis (Y axis correction results in undesired flight characteristics).
- Engines are now fully separated, they no longer act as a one unit.
- Touchdown points so landing gear is no longer below the ground.
- Keyboard shortcuts to call switches instead of doing work.
- APU sounds are separated.
### Fixed
- Working stuff sound not played when loading an existing scenario.
- Oxidizer pumps not turning off if the fuel line is closed.
- Messing OMS and RCS fuel pump sounds.
- Extend/Retract switches not switching back to original position if moved to the current position.
- Hydraulic pressure not correctly aligned in the EICAS.
- Wing position changes (although not completely) if APU isn't on.
- Oxidizer flow showing 0.
- VC seat position not saved.
- Fuel pressures not shown when the engine is not working.
- Reduced the tick sound in the afterburner sound.
- Wing position not correctly read from scenario.

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
- RCS and OMS are now separated systems.
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