
# Meteorology Station

### For: happy-gecko-efm32

## Usage guide 
### Main Layout
This project consists of multiple pages:

 - Clock Page
 - Weather Page
 - Clock Adjust Page
 - Weather Adjust Page
 - General Menu

On startup the default page is the clock page. Default time is set to the unix epoch (01.01.1970). 
The general menu is used to switch through pages:

 - PB1 is the select/set/act button in general. Use the PB1 button on the Clock Page or the Weather Page to open the General Menu. On the General Menu page pressing the PB1 button will select and open the selected page from the menu. [TO_DO] When adjusting the time of the Clock Adjust Page the PB1 button is used to cycle through the values of time parts, except for the year part where it is used to decrement the year, and the Confirm/Exit part where it is used to select the desired option.
 - PB0 is the switch button. On the General Menu page it can be used to switch through options. [TO_DO] When adjusting the time of the Clock Adjust Page the PB0 button is used to switch through the time parts, except for the year part where it is used to increment the year.
 - PB0 & PB1 can be pressed together to confirm the adjusted year on the Clock Adjust Page.

Pages can easily be added (although admittedly they should have been defined in an enum type, instead of just numbers), as well as menu options.
