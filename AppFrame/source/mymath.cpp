#include "mymath.h"

#define TwoPI   (PI * 2.0f)
#define PIOver2 (PI / 2.0f)

namespace mymath
{

    // ƒC[ƒWƒ“ƒO
    float EasingLinear(float cnt, float start, float end, float frames)
    {
        return (end - start) * cnt / frames + start;
    }
    float EasingInQuad(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt + start;
    }
    float EasingOutQuad(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return -(end - start) * cnt * (cnt - 2) + start;
    }
    float EasingInOutQuad(float cnt, float start, float end, float frames)
    {
        cnt /= static_cast<float>(frames) / 2.0f; // C³: doubleƒŠƒeƒ‰ƒ‹‚ğfloat‚É
        if(cnt < 1)
        {
            return static_cast<float>((end - start) / 2.0f * cnt * cnt + start); // C³: 2.0 ¨ 2.0f
        }
        cnt--;
        return static_cast<float>(-(end - start) / 2.0f * (cnt * (cnt - 2) - 1) + start); // C³: 2.0 ¨ 2.0f
    }
    float EasingInCubic(float cnt, float start, float end, float frames)
    {
        cnt /= static_cast<float>(frames);
        return (end - start) * cnt * cnt * cnt + start;
    }
    float EasingOutCubic(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return static_cast<float>((end - start) * (cnt * cnt * cnt + 1) + start);
    }
    float EasingInOutCubic(float cnt, float start, float end, float frames)
    {
        cnt /= static_cast<float>(frames) / 2.0f;
        if(cnt < 1)
        {
            return static_cast<float>((end - start) / 2.0f * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return static_cast<float>((end - start) / 2.0f * (cnt * cnt * cnt + 2) + start);
    }
    float EasingInQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt * cnt * cnt + start;
    }
    float EasingOutQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return -(end - start) * (cnt * cnt * cnt * cnt - 1) + start;
    }
    float EasingInOutQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return static_cast<float>((end - start) / 2.0f * cnt * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return static_cast<float>(-(end - start) / 2.0f * (cnt * cnt * cnt * cnt - 2) + start);
    }
    float EasingInQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt * cnt * cnt * cnt + start;
    }
    float EasingOutQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return (end - start) * (cnt * cnt * cnt * cnt * cnt + 1) + start;
    }
    float EasingInOutQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return static_cast<float>((end - start) / 2.0f * cnt * cnt * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return static_cast<float>((end - start) / 2.0f * (cnt * cnt * cnt * cnt * cnt + 2) + start);
    }
    float EasingInSine(float cnt, float start, float end, float frames)
    {
        return static_cast<float>(-(end - start) * cos(cnt / frames * (PIOver2)) + end + start);
    }
    float EasingOutSine(float cnt, float start, float end, float frames)
    {
        return static_cast<float>((end - start) * sin(cnt / frames * PIOver2) + start);
    }
    float EasingInOutSine(float cnt, float start, float end, float frames)
    {
        return static_cast<float>(-(end - start) / 2.0 * (cos(PI * cnt / frames) - 1) + start);
    }
    float EasingInExpo(float cnt, float start, float end, float frames)
    {
        return static_cast<float>((end - start) * pow(2.0, 10 * (cnt / frames - 1)) + start);
    }
    float EasingOutExpo(float cnt, float start, float end, float frames)
    {
        return static_cast<float>((end - start) * (-pow(2.0, -10 * cnt / frames) + 1) + start);
    }
    float EasingInOutExpo(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return static_cast<float>((end - start) / 2.0 * pow(2.0, 10 * (cnt - 1)) + start);
        }
        cnt--;
        return static_cast<float>((end - start) / 2.0 * (-pow(2.0, -10 * cnt) + 2) + start);
    }
    float EasingInCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return -(end - start) * static_cast<float>(sqrt(1 - cnt * cnt) - 1) + start;
    }
    float EasingOutCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return static_cast<float>((end - start) * sqrt(1 - cnt * cnt) + start);
    }
    float EasingInOutCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return static_cast<float>(-(end - start) / 2.0 * (sqrt(1 - cnt * cnt) - 1) + start);
        }
        cnt -= 2;
        return static_cast<float>((end - start) / 2.0 * (sqrt(1 - cnt * cnt) + 1) + start);
    }
}