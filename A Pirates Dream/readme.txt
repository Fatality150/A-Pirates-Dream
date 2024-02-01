Changes made:

	game.h

Added a player health variable on line 61

Added a AudioManager on line 64

Added an varable to store the sound file index on lines 67 and 70

	game.cpp

Initialized player health on line 80

Added new loop to spawn 3 game objects (enemies) starting on line 125 to 129

Set the background game object to not active as to not collide with it

Changed the texture paths on line 179 to include new sprites for the player and enemies

Changed control handler to allow for a slightly more comfortable movement system. Lines 247 to 275

Added detection for player explosion and subsequent game closure from lines 286 to 302

Added a checker for explosion timers from lines 330 to 341

Added collision handler on lines 345 to 378

	game_object.h
	
Added a timer object on line 62

Added a getter and a setter for the timer on lines 38 and 52

Added an active member to game object to detect if the are valid to collide with on line 65

Added a getter and a setter for object activeness on lines 37 and 51

	game_object.cpp

Initialized the timer on line 21

Deleted the timer on line 26

Used the timer class functions to pass the getter value on starting from line 29

Used the timer class functions to start the time starting from line 35


	timer.h

Added variables for the start time, end time, and wether the timer is active from line 24

	timer.cpp

Initialized the active member on line 10 

Set start time, end time and activeness on lines 23 to 29

Checked wether the timer is finished on lines 36 to 42



			Made On Windows 11