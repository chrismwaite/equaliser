# Equaliser
Equaliser is a watch face for the Pebble Time

http://apps.getpebble.com/en_US/application/558593636b9455d89e0000f0

It has the following features:

* Battery level display
* Day of the week and date
* Bluetooth connectivity indicator (with vibrate on connect/disconnect)
* Support for 12 and 24 hour time format
* Ability to disable the second hand via the configuration page (to preserve battery life)

How does it work?

Rows fill up from the bottom to the top - Each complete row represents an hour that has passed since midnight.
Columns fill up from left to right - Each column represents a minute that has passed since the previous full hour.
Seconds are represented by the orange line that moves horizontally across the screen. When it reaches the end, a new column is filled in to indicate a complete minute.

There are a few markers to assist with readability. Orange blocks are displayed down the left hand side at every 2 hour increment. The orange horizontal marker represents midday. The black vertical markers are 15 minute increments with the thickest being half past the hour.

Example: 3 rows are filled. A 4th is partially filled just past the first vertical black line. The time is therefore just gone 3:15am.

I hope you enjoy it :)
