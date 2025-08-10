# MY Player Notes

Time syncing and keeping: custom parsing

## Implementation of looking into the future
Option A: External tick generator parallel to the one inside each MD_Track
- "Easy" to implement without accessing internal MD_Track workings. 
- Might cause the timers to go out of sync.
- Track has to be played until + 4 bars and captured.

    // I need to look 4 bars into the future in the file to enable looping and to be able to highlight the keys one bar ahead.
    // This is tricky to do as midi events come as a stream of events with a start and an unknown end. 
    // To find the end of a key press you actually have to parse all the way to the key up event.
    // A pointer to this structure type is passed to the callback function registered using setMidiHandler().
    // No note is longer than 4 bars, so if i look 4 bars into the future i should know the duration of every note.
