# Dynam EQ: an AI-backed equalizer for optimal music listening

DynaEQ is an equalizer that will use AI to automatically adjust your computer's audio equalization settings based on the genre of music you're listening to.

## Stack:
 - C (lol)

## Current state (v0):
 - Manual mapping to limited genres obtained from playerctl metadata xesam:genre field
 - Pulling track metadata from playerctl using taglib - this will later be passed to the AI model
 - Audio equalization adjustments using easyeffects

## Future additions:
 - Applying AI-based equalization adjustments using third-party API calls
 - Train my own model to make good decisions for equalization adjustments based on audio signatures(?)
 - Extracting audio signatures for songs from third-party APIs to inform equalization adjustments
 - Make streamed content available for equalization adjustments
