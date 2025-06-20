# Documentation Index

Complete documentation for the Bobcat Ignition Controller project.

## Quick Start

1. **[Main README](../README.md)** - Project overview and features
2. **[Hardware Overview](hardware_overview.md)** - Complete hardware specifications
3. **[Wiring Guide](wiring_guide.md)** - Step-by-step installation instructions
4. **[Flashing Firmware](flashing_firmware.md)** - Programming the controller

## System Architecture

### Core Documentation
- **[System Control Diagram](system_control_diagram.md)** - State machine and control logic
- **[GPIO Connections](gpio_connections.md)** - Pin assignments and interface specifications
- **[Board Pinout](board_pinout.md)** - Complete LILYGO T-Relay pin reference
- **[Schematics](schematics.mmd)** - System diagrams and block diagrams

### Hardware Reference
- **[Hardware Overview](hardware_overview.md)** - Complete hardware specifications and requirements
- **[Wiring Guide](wiring_guide.md)** - Installation and wiring instructions
- **[Board Pinout](board_pinout.md)** - GPIO assignments and electrical specifications

### Development
- **[Board Development](board_development.md)** - Development setup and build instructions
- **[TODO](TODO.md)** - Future improvements and known issues

## Documentation Structure

### Primary Documents
These documents provide the essential information for understanding and using the system:

1. **[README](../README.md)** - Entry point with project overview
2. **[Hardware Overview](hardware_overview.md)** - Comprehensive hardware reference
3. **[Wiring Guide](wiring_guide.md)** - Complete installation guide
4. **[System Control Diagram](system_control_diagram.md)** - System behavior and logic

### Reference Documents
Detailed technical references for development and troubleshooting:

1. **[GPIO Connections](gpio_connections.md)** - Pin assignments and interface details
2. **[Board Pinout](board_pinout.md)** - Complete pin reference and specifications
3. **[Schematics](schematics.mmd)** - System diagrams (Mermaid format)

### Development Documents
Information for developers and advanced users:

1. **[Board Development](board_development.md)** - Development environment setup
2. **[Flashing Firmware](flashing_firmware.md)** - Programming instructions
3. **[TODO](TODO.md)** - Future enhancements and known issues

## Key System Features

### Current Implementation (4-Relay Design)
- ✅ **Main Power Control** - Master electrical system switch
- ✅ **Glow Plug Control** - 20-second automated preheating
- ✅ **Starter Control** - Momentary starter activation
- ✅ **Combined Lighting** - Single relay for all work lights
- ✅ **Web Interface** - Mobile-responsive control panel
- ✅ **Sensor Monitoring** - Temperature, pressure, voltage, fuel level
- ✅ **Safety Alerts** - Visual warnings (no auto-shutdown)
- ✅ **Manual Engine Stop** - Engine stop via manual lever only

### Hardware Platform
- **Controller**: LILYGO T-Relay ESP32 (4-channel relay board)
- **Relays**: 4x SPDT relays, 10A capacity each
- **Sensors**: 4x analog inputs (12-bit ADC)
- **Status**: 2x digital inputs for system feedback
- **Communication**: WiFi access point (192.168.4.1)
- **Power**: 12V input with onboard regulation

## Document Relationships

```
README.md (Project Overview)
├── Hardware Overview → Complete hardware specifications
├── Wiring Guide → Installation instructions
├── System Control → Logic and behavior
└── Development Docs
    ├── Board Development → Setup and build
    ├── Flashing Firmware → Programming
    └── Reference Docs
        ├── GPIO Connections → Pin assignments
        ├── Board Pinout → Electrical specifications
        └── Schematics → System diagrams
```

## Documentation Standards

### Format Guidelines
- **Markdown**: All documentation uses Markdown format
- **Diagrams**: Mermaid format for technical diagrams
- **Code**: Syntax highlighting for code blocks
- **Tables**: Structured data presentation
- **Links**: Internal cross-references between documents

### Content Standards
- **Accuracy**: All information verified against actual hardware/software
- **Completeness**: Sufficient detail for implementation
- **Clarity**: Written for multiple skill levels
- **Currency**: Kept up-to-date with code changes

### File Organization
- **docs/**: All documentation files
- **README.md**: This index file
- ***.md**: Individual documentation files
- ***.mmd**: Mermaid diagram files

## Viewing Mermaid Diagrams

The schematic diagrams use Mermaid format. To view them:

### VS Code (Recommended)
1. Install "Mermaid Preview" extension
2. Open `.mmd` file
3. Use Ctrl+Shift+P → "Mermaid Preview"

### Online Viewers
- [Mermaid Live Editor](https://mermaid.live/)
- [GitHub's built-in Mermaid support](https://github.blog/2022-02-14-include-diagrams-markdown-files-mermaid/)

### Local Tools
- Mermaid CLI: `npm install -g @mermaid-js/mermaid-cli`
- Generate images: `mmdc -i schematics.mmd -o schematics.png`

## Contributing to Documentation

### Making Changes
1. Update the relevant documentation file
2. Verify all cross-references remain valid
3. Test any code examples or procedures
4. Update this index if adding new documents

### Quality Checklist
- [ ] Information is accurate and tested
- [ ] Formatting follows Markdown standards
- [ ] Links work correctly
- [ ] Diagrams render properly
- [ ] Content is clear and complete

## Support and Maintenance

### Keeping Documentation Current
- Review documentation when making code changes
- Update hardware specifications if components change
- Verify wiring information with any board changes
- Test all procedures and examples

### Issue Reporting
For documentation issues:
1. Check if information is outdated
2. Verify against actual hardware/software
3. Update the affected document
4. Cross-check related documents for consistency

## Version History

### Latest Version (Current)
- Complete 4-relay implementation
- Comprehensive hardware documentation
- Detailed wiring guide
- Updated system diagrams
- Removed obsolete references

### Previous Versions
- Initial implementation with 5-relay design
- Legacy documentation (removed)
- Old development notes (archived)
5. **Operation**: [system_operation.md](system_operation.md) - How to use the system
6. **Web Interface**: [web_interface.md](web_interface.md) - Using the control panel

---

## 🔄 Document Maintenance

This documentation is maintained to reflect the current system implementation:
- **Hardware**: 4-relay LILYGO T-Relay ESP32 board
- **Target**: Old Bobcat equipment with manual engine stop
- **Software**: Web-based control interface with safety monitoring
- **Last Updated**: June 2025
