# Meteorology Station for EFM32 Happy Gecko

A feature-rich meteorology station application designed for the Silicon Labs EFM32 Happy Gecko microcontroller, featuring an interactive LCD display with clock and weather monitoring capabilities.

## Overview

This embedded C project transforms the EFM32 Happy Gecko development board into a fully functional meteorology station with a user-friendly multi-page interface. The application provides real-time clock functionality with time adjustment capabilities and environmental monitoring through integrated humidity and temperature sensors.

## Features

- **Real-time Clock**: Accurate timekeeping with customizable time settings
- **Weather Monitoring**: Temperature and humidity tracking
- **Interactive LCD Display**: Multi-page graphical interface with custom fonts
- **Button-based Navigation**: Intuitive two-button control system
- **Custom Graphics Engine**: Efficient display rendering with 7-segment font support
- **Page-based Architecture**: Easily extensible page system

## Project Structure

```
happy-gecko-efm32-clk/
├── src/
│   ├── 7segment_font.c       # Custom 7-segment display font definitions
│   ├── clock_control.c        # Real-time clock management and timekeeping
│   ├── clock_control.h        # Clock control interface definitions
│   ├── extra_fonts.h          # Additional font declarations
│   ├── font_custom.c          # Custom font implementations
│   ├── graphics.c             # Main graphics rendering engine
│   └── humitemp.c            # Humidity and temperature sensor interface
├── includes/                  # Header files and library includes
├── service/                   # Service layer components
├── external_copied_files/     # External dependencies
├── external_copied_files_inc/ # External include files
├── .cproject                  # Eclipse CDT project configuration
├── .project                   # Eclipse project metadata
└── README.md                  # This file
```

## 🖥️ User Interface

### Available Pages

The application consists of five interactive pages:

1. **Clock Page** - Displays current time in a clear format
2. **Weather Page** - Shows temperature and humidity readings
3. **Clock Adjust Page** - Interface for modifying time settings
4. **Weather Adjust Page** - Configuration page for weather settings
5. **General Menu** - Central navigation hub for page selection

### Navigation Controls

#### Button Functions

**PB1 (Select/Action Button)**
- On Clock/Weather Page: Opens the General Menu
- On General Menu: Selects and opens the highlighted page
- On Clock Adjust Page: Cycles through time component values (hours, minutes, seconds)
- For year adjustment: Decrements the year value
- On Confirm/Exit: Executes the selected option

**PB0 (Switch Button)**
- On General Menu: Cycles through available menu options
- On Clock Adjust Page: Switches between time components (hours, minutes, seconds, year)
- For year adjustment: Increments the year value

**PB0 + PB1 (Combined)**
- On Clock Adjust Page: Confirms the adjusted year value

### Default Settings

- **Initial Page**: Clock Page
- **Default Time**: Unix Epoch (January 1, 1970, 00:00:00)

## Getting Started

### Prerequisites

- Silicon Labs EFM32 Happy Gecko Starter Kit
- Simplicity Studio IDE (or Eclipse CDT with ARM toolchain)
- J-Link debugger (typically integrated on the starter kit)
- Silicon Labs SDK/EMLIB

### Hardware Requirements

- EFM32 Happy Gecko Board (EFM32HG322 or compatible)
- LCD display (integrated on most starter kits)
- Temperature/Humidity sensor (Si7013 or compatible)
- Push buttons PB0 and PB1

### Building the Project

1. **Clone the repository**
   ```bash
   git clone https://github.com/Krikchou/happy-gecko-efm32-clk.git
   cd happy-gecko-efm32-clk
   ```

2. **Open in Simplicity Studio or Eclipse**
   - Import existing project
   - Select the project directory
   - The `.project` and `.cproject` files will be automatically detected

3. **Build the project**
   - Right-click project → Build Project
   - Or use the build button in the IDE toolbar

4. **Flash to device**
   - Connect your Happy Gecko board via USB
   - Right-click project → Debug As → Silicon Labs ARM Program
   - The application will be flashed and started automatically

## Configuration

### Adding New Pages

The page system is designed for easy extensibility:

1. Define a new page rendering function in `graphics.c`
2. Add the page to the menu structure
3. Implement button handlers for the new page
4. Update the page navigation logic

**Note**: The current implementation uses numeric page identifiers. Consider refactoring to use enumerated types for improved code maintainability.

### Customizing Display

- **Fonts**: Modify `7segment_font.c` and `font_custom.c` for custom characters
- **Graphics**: Update `graphics.c` to change rendering behavior
- **Layout**: Adjust positioning constants in the graphics engine

## Technical Details

### Key Components

**Clock Control (`clock_control.c`)**
- Manages RTC (Real-Time Clock) peripheral
- Handles time calculations and updates
- Provides time adjustment interface

**Graphics Engine (`graphics.c`)**
- LCD display driver interface
- Page rendering and composition
- Custom font rendering
- UI element drawing

**Humidity/Temperature Sensor (`humitemp.c`)**
- I2C communication with environmental sensor
- Data acquisition and conversion
- Sensor calibration routines

## Known Issues & TODOs

- [ ] Implement complete time adjustment for all components on Clock Adjust Page
- [ ] Refactor page identifiers from integers to enum types
- [ ] Complete Weather Adjust Page functionality
- [ ] Add error handling for sensor communication failures
- [ ] Implement power-saving modes

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## License

This project is open source. Please check with the repository owner for specific licensing terms.

## Resources

- [Silicon Labs EFM32 Happy Gecko Documentation](https://www.silabs.com/mcu/32-bit/efm32-happy-gecko)
- [EFM32 SDK Documentation](https://docs.silabs.com/)
- [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio)

## Authors

**Krikchou**
- GitHub: [@Krikchou](https://github.com/Krikchou)
**аdrianп59**
- GitHub: [@adrianp59](https://github.com/adrianp59)
---

*This project demonstrates embedded C programming, microcontroller peripheral control, sensor integration, and user interface design on resource-constrained hardware.*