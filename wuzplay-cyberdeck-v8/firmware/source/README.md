# Firmware source reconstruction status

The working v8 reproduction is **binary-first**: the verified Nordic DFU payload is the gold firmware anchor.

The original exact source tree that produced `pixljs.bin` SHA-256 `722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad` has not been recovered, so this project does not pretend an unrelated stock source tree reproduces it.

Historical custom pixl.js reference points include:

- combined flip-screen + return-key snapshot: `74a505a948a1cfedd296b0033239f74b4090c420`
- later return-key-only head: `63379c3c4c803fecc59f3a12f3a7f6b2105e12aa`
- later flip-screen head: `0a9069a1d2e81bf713f8cf73b854c07ef8b48084`

None is asserted to be the exact v8 binary source. Reconstruction rule: preserve the verified binary, port only proven custom behavior, and never silently replace it with a default feature set.
