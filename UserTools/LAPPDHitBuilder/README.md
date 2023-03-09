# LAPPDHitBuilder

LAPPDHitBuilder saves LAPPDPulse and LAPPDHit information taken from other upstream tools and adds location information, then saves this to an output ANNIEEvent binary file with PsecData replaced by LAPPDPulse and LAPPDHit maps. This will need a fair bit of future work for the full LAPPD hit reconstruction, but it's a start.

## Data

**CFDRecoLAPPDPulses** `map<unsigned long, vector<LAPPDPulse>>`
* Takes this data from the `ANNIEEvent` store and finds the number of peaks

**LAPPDHits** `map<unsigned long, vector<LAPPDHit>>`

## Configuration

Describe any configuration variables for LAPPDPulseBuilder.

```
param1 value1
param2 value2
```
