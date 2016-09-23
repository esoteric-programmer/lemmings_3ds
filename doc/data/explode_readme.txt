explode.dat contains the raw bytes extracted from the DOS EXE.  Each particle 
position is represented in the file by two signed bytes.  (A signed byte 
ranges from -128 to 127, with hex 0x80 being -128 and 0xFF being -1.)  The 
first of the two bytes is the offset x from the lemming's x position, and the 
second is the offset y.

In the file, the first 160 bytes contains the positions for particles 1 thru 
80, for when the lemming's animation index is 1 (remember that when an 
exploding lemming's animation index is 0, we draw the "explosion" graphic 
rather than the particles).  Then the next 160 bytes contains the positions 
for particles 1 thru 80 when the lemming's animation index is 2, and so forth.
One more special thing:  whenever a particle's position offsets, as stored in 
the file, becomes (-128, -128), this means the particle has vanished and 
would no longer be displayed for the current animation frame and hereafter.

Finally, the colors of the particles are specified in another lookup table in 
the EXE.  A color value is of course an index to the palette used by the
current level.  The color assignment cycles through every 16 particles, as 
follows:

particle 1: 4
particle 2: 15
particle 3: 14
particle 4: 13
particle 5: 12
particle 6: 11
particle 7: 10
particle 8: 9
particle 9: 8
particle 10: 11
particle 11: 10
particle 12: 9
particle 13: 8
particle 14: 7
particle 15: 6
particle 16: 2
...

As an example, consider the first 8 bytes (1 - 8) of the file:

  -52, -100, -23, -47, 7, -25, -2, -21, ...

Now jump ahead and consider bytes 161 - 168:

  -128, -128, -47, -93, 15, -53, -4, -44, ...

and then bytes 321 - 328:

  -128, -128, -128, -128, 23, -80, -6, -67, ...

481 - 488:

  -128, -128, -128, -128, 30, -107, -7, -90, ...

641 - 648:

  -128, -128, -128, -128, -128, -128, -9, -112, ...

The numbers shown above gives you the particle trails for particles #1 thru #4.
Suppose the lemming is at (100, 100). The frame-by-frame positions for those
particles start off as follows:

particle 1: (48, 0), <gone>
particle 2: (77, 53), (53, 7), <gone>
particle 3: (107, 75), (115, 47), (123, 20), (130, -7) [not displayed as it's off-screen], <gone>
particle 4: (98, 79), (96, 56), (94, 33), (93, 10), (91, -12) [not displayed as it's off-screen], ...

Notice how (-128, -128) signals the end of a particle.
