#ifndef SKIP_HPP
#define SKIP_HPP

struct Skip {
    Skip(float absBeat, float skipTime, float beatDuration) : absBeat(absBeat), skipTime(skipTime), beatDuration(beatDuration), beatEnd(absBeat + beatDuration) {}

    float absBeat;

    float skipTime;
    float beatDuration;

    float beatEnd;
};

#endif // STOP_HPP