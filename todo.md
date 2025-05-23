# TB1 - Task list
Feature expansions and bugfixes for the TB1 project are tracked in this document

## Feature Expansions
List of features that should be added or expanded upon.

### Keyboard playing
The synth should be playable from the computer keyboard. Notes should preferably be mapped in the same way as the "musical keyboard" in Logic Pro.

### Update filter type
Add a more advanced filter type. This could be an MS20 style filter or some sort of Moog style ladder filter perhaps.

### GUI rework
The GUI is getting crowded and can be improved.

### Click-and-hold playback mode
Notes played only when the mouse is clicking them, rather than toggling.

### Polyphonic mode
Add a mode for polyphonic playing.

### In Progress - Arp modes
Currently notes are being played in the order they have been selected. The following modes should be added:
- Up
- Down
- Up/Down
- Random (?)

### Add clipping/drive/saturation
There is some saturation at the filter stage(?) currently. Add some saturation control to add some harmonics to the tone of the synth.

### Delay effect
Add delay effect.

## Improvments and reworks
List of reworks of existing functionallity to improve upon it

### Rework signal processing flow
The signal 'flow' from oscillator via pitch and envelope to the filters could be reworked.
It is somewhat convoluted at the momement and not modular.

### Fine tune slider min/max values
Sliders aren't really useful in their full value ranges. Finetune min/max values for each slider so that they *feel* better.

## Bugs
List of known bugs that need to be addressed.

### add_arp_note is constantly called in single note mode
When in single note mode function add_arp_note is constantly called since there is no check for "re-toggling" of the note.

### Resonance is barely noticable
Confirm that resonance is working as intended. Currently there is a barely noticable change when moving the resonance slider.

## Completed tasks
List of completed changes. It will be cleared out every once in a while.

### LFO
Implement a LFO to control the filter or resonance.

### Arp tempo
Add tempo control to the arp.

### Hold control for note length in arp mode
Add some way to control the note length while in arp mode. Currently notes are held until the note switches.
This makes the release section of the envelope useless.

### Buttons look "squished"
The buttons for wave type and playback mode are rendering incorrectly. They look kinda "squished", like the button outline has been squished down into a strikethrough of the button label.

