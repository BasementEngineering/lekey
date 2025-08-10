# MY Player Notes

Time syncing and keeping: custom parsing

## Implementation of looking into the future
Option A: External tick generator parallel to the one inside each MD_Track
- "Easy" to implement without accessing internal MD_Track workings. 
- Might cause the timers to go out of sync.
- Track has to be played until + 4 bars and captured.
