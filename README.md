#Shooter-Serial-Control
Shooter-Serial-Control controls the two-cylinder, four-relay pneumatic loading and firing system via a serial connection.

##Input
Input is solely numerical. Certain numbers 1 through 9 activate different actions.
Certain numbers 11 through 19 (corresponding to the 1-9 commands) expect to be followed by a time in milliseconds. The Arduino will then wait the specified length of time before performing the action.

##Feedback
Various punctuation marks are placed in feedback.
A '..' after feedback indicates that the action stated will complete promptly.
A '...' before feedback indicates that the action stated follows an automatic action.
A '...' after feedback indicates that an automatic action will follow the stated action.
A '!!' after feedback indicates that no action was performed due to user error.

##Troubleshooting
If the Arduino is not responding to commands, ensure that Newline is enabled in the serial connection.
