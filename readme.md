# 6player
A simple and accessible audio player based on BASS library

# This project is no longer maintained
There have been no changes since a few years now. 6player is in C++03, and using direct Win32 API for its interface.
It's time to start over with new tools, new ideas. Check out my new project [Sample](https://github.com/qtnc/sample).
This repository is kept for archive.


## Features
* Support playing of about 90 popular and less popular formats; most poupular includes OGG, MP3, AAC, WMA, flac, MIDI, IT/XM/S3M/MOD, ... also plays audio part of video files (AVI, WMV, FLV, 3GP, OGV, ...); thank to BASS extensibility mechanism
* Support of 5 playlist formats: PLS, M3U, ASX, WPL, XSPF
* Play streams from URL, support for HTTP, HTTPS and FTP; includes a database of 20000 internet radio stations
* Convert a single file or export the entire playlist to OGG, MP3, AAC, flac, wave, etc.
* Quickly search within your playlist thank to the quick filter 
* Edit tags of MP3 and OGG files
* Change pitch, speed, rate, and add a variety of effects in real time
* Support microphone, and effects can also be applied to it
* Broadcast your music and your voice + optional effects to shoutcast, icecast, or private HTTP BASS server
* Coming soon: lua scripting, and web HTTP control server

## License
6player is distributed under GPL license. 
If you want to contribute, please contact me using the contact page at <http://quentinc.net/>

## Keyboard shortcut list
- C, space or numpad 5: play/pause
- X or numpad 0: restart play
- K or numpad 7: previous song
- L or numpad 9: next song
- DEL: remove current song from playlist and go to next one
- I or numad 1: loop on/off
- P or numpad 3: random on/off
- M: cross fade on/off (cross fade is 6 seconds long)
- A and Q: adjust speed, without changing pitch
- S and W: adjust pitch in semitones, without changing speed
- Shift+A/Q: adjust rate (= speed and pitch simultaneously)
- Shift+S/W: adjust pitch by steps of 0.1 semitone, without changing speed
- D and E: adjust equalizer 1, 300Hz
- F and R: adjust equalizer 2, 700Hz
- G and T: adjust equalizer 3, 1.5kHz
- H and Z: adjust equalizer 4, 3.2kHz
- J and U: adjust equalizer 5, 7kHz
- O: disable all pitch/speed/rate effects
- Up/down or numpad 8/2: adjust global volume
- Left/right: navigation 5 seconds
- Shift+Up/Down or Page up/down: navigation 30 seconds
- Shift+Page up/down: navigation 5 minutes
- F3 or Alt+3: show properties of current file
- Ctrl+O: open file
- Ctrl+Shift+O: open file and append to playlist
- Ctrl+U: open URL
- Ctrl+Shift+U: Open URL and append to playlist
- Ctrl+D: open directory
- Ctrl+Shift+D: open directory and append to playlist
- Ctrl+S: save as / convert
- Ctrl+Shift+S: save playlist
- Ctrl+J: time jump / go to position
- Ctrl+P: options dialog
- Alt+P: open playlist window
- Alt+R: open radios window
- Alt+M: open MIDI panel window
- F4: turn mic on/off
- F6: configure audio devices

Multimedia keys and commands on headphones :

- Play: play/pause
- Next/prev: go to next/previous song 
- Hold next/prev: navigate by 5 seconds every 250ms
- Long play, then next/prev: adjust pitch
- Long play, then hold next/prev: adjust speed
- Very long play: reset rate/pitch/speed to default

Long = between 250ms and 1s, very long = more than 3s
