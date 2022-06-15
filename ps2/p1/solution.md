### P1 Solution

## 1.
The VGA hardware provides the ability to specify a horizontal division which divides the screen into two windows which can start at separate display memory addresses. The horizontal division of the scan line address is specified by the Line Compare field, thus it can be used to separate the screen into two distinct regions.

### necessary register value settings:  
1. Set the Line Compare field as the end of main window display for split-screen operation, since the current scan line start address is reset to 0 when the line counter reaches the value in the Line Compare Register:

    bit 9 is in the Maximum Scan Line Register (6th bit), bit 8 is in the Overflow Register (4th bit), and bits 7-0 are in the Line Compare Register

2. Set Preset Row Scan field to achieve scolling for the main window (displayed using top window), use bottom window to display the status bar. Since the bottom window has an effective Preset Row Scan value of 0, it will automatically be non-scrolling. The value of Preset Row Scan field specifies how many scan lines to scroll the display upwards. 

3. Set Start Address High Register and Start Address Low Register as the start of main window display.

4. Set Offest Register to provide for virtual widths.

5. (Optional) Set Pixel Panning Mode to allow synchronized panning.

### constraints:  
1. the bottom window's starting display memory address is fixed at 0

2. either both windows are panned by the same amount, or only the top window pans

3. Preset Valid values of Preset Row Scan field range from 0 to the value of the Maximum Scan Line field.

## 2. 
To write a palette entry, output the palette entry's index value to the DAC Address Write Mode Register then perform 3 writes to the DAC Data Register, loading the red, green, then blue values into the palette RAM. 
