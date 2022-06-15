### P2 Solution

#### A

- **MTCP_BIOC_ON:** Enable Button interrupt-on-change. It allows interrupt to occur when there is a change in states. 
  - **When**: The button's status is changing
  - **Effect**: Allow interrupt to occur when the button's status is changing.
  - **Returned Message**: MTCP_ACK, which indicates the MTC successfully 		completes a command.

- **MTCP_LED_SET:** Put the LED display into user-mode. In this mode, the value 		specified by the MTCP_LED_SET command is displayed.
  - **When**: When we want to display LED values on the screen
  - **Effect**: the value specified by the MTCP_LED_SET command is displayed.
  - **Returned Message**: MTCP_ACK

#### B

- **MTCP ACK:** 
  - **When**: A command from computer is executed.
  - **Information**: Indicates the MTC successfully completes a command.
- **MTCP BIOC EVENT**: 
  - **When**: the Button Interrupt-on-change mode is enabled.
  - **Information**: Indicates the status of the button, whether it is pressed or released.
- **MTCP_RESET**:
  - **When**: The device power up, a RESET button press, or an MTCP_RESET_DEV command.
  - **Information**: Indicates the device re-initializes itself. 

#### C
