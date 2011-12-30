
"Piano Protagonist" Notes:

When the game launches, the main menu should allow the player to choose  "play" or "options".  If he presses "play", a song chooser (directory browser) screen should be displayed.  Once he picks a song, a song options screen should be displayed.

The song options screen should display the list of tracks in the song.  The player can mark each track as "player", "accompaniment", or "off".  The player can also set a tempo modifier between 1% and 200%.  A "preview" button should display the main game screen, but the computer will play tracks marked as "player" in addition to the "accompaniment".  A "play" button takes the player to the main game screen.

The main game screen should be a piano roll displayed in 3D such that the pitch axis goes from left to right (like a piano keyboard), and time increases going into the screen and slightly upward (fading in the distance).  The time is displayed linearly in seconds (i.e., if the tempo varies over the course of a song, beats are drawn closer together or farther apart).  A grid of pitches (possibly with piano key coloring) and beats is displayed at all times.  The grid is bisected by spheres corresponding to each note which the player is intended to play.  As the song plays, the notes and grid lines should scroll towards the viewer, out of the screen.  When notes reach the "current time" at the front of the screen (which has a visual indicator), they are removed from the display.

The computer will play back the tracks of the song marked "accompaniment", and the player is expected to play the tracks marked "player", whose notes are displayed in the piano roll.  When the player hits correct notes at approximately the correct time, his score increases.  (More complicated scoring possibilities will be explored later.)

The main options screen should let the player pick a screen resolution, and MIDI input and output ports, each from a list.  These choices, plus the most recently used song directory and the high scores should be stored in the registry.

