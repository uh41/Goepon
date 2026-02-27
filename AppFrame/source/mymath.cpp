#include "mymath.h"

#define TwoPI   (PI * 2.0f)
#define PIOver2 (PI / 2.0f)

namespace mymath
{

    // イージング
    
    // 常に加速度が一定で緩急のないイージング
    float EasingLinear(float cnt, float start, float end, float frames)
    {
        return (end - start) * cnt / frames + start;
    }

	// 加速していくイージング
    float EasingInQuad(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt + start;
    }

	// 減速していくイージング
    float EasingOutQuad(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return -(end - start) * cnt * (cnt - 2) + start;
    }
    
	// 加速してから減速していくイージング
    float EasingInOutQuad(float cnt, float start, float end, float frames)
    {
        cnt /= StCas<float>(frames) / 2.0f; 
        if(cnt < 1)
        {
            return StCas<float>((end - start) / 2.0f * cnt * cnt + start);
        }
        cnt--;
        return StCas<float>(-(end - start) / 2.0f * (cnt * (cnt - 2) - 1) + start); 
    }

	// 加速がさらに強いイージング
    float EasingInCubic(float cnt, float start, float end, float frames)
    {
        cnt /= StCas<float>(frames);
        return (end - start) * cnt * cnt * cnt + start;
    }

	// 減速がさらに強いイージング
    float EasingOutCubic(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return StCas<float>((end - start) * (cnt * cnt * cnt + 1) + start);
    }

	// 加速と減速がさらに強いイージング
    float EasingInOutCubic(float cnt, float start, float end, float frames)
    {
        cnt /= StCas<float>(frames) / 2.0f;
        if(cnt < 1)
        {
            return StCas<float>((end - start) / 2.0f * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return StCas<float>((end - start) / 2.0f * (cnt * cnt * cnt + 2) + start);
    }

	// 加速と減速がさらにさらに強いイージング
    float EasingInQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt * cnt * cnt + start;
    }

	// 減速がさらにさらに強いイージング
    float EasingOutQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return -(end - start) * (cnt * cnt * cnt * cnt - 1) + start;
    }

	// 加速と減速がさらにさらに強いイージング
    float EasingInOutQuart(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return StCas<float>((end - start) / 2.0f * cnt * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return StCas<float>(-(end - start) / 2.0f * (cnt * cnt * cnt * cnt - 2) + start);
    }

	// 加速と減速がさらにさらにさらに強いイージング
    float EasingInQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return (end - start) * cnt * cnt * cnt * cnt * cnt + start;
    }

	// 減速がさらにさらにさらに強いイージング
    float EasingOutQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return (end - start) * (cnt * cnt * cnt * cnt * cnt + 1) + start;
    }

	// 加速と減速がさらにさらにさらに強いイージング
    float EasingInOutQuint(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return StCas<float>((end - start) / 2.0f * cnt * cnt * cnt * cnt * cnt + start);
        }
        cnt -= 2;
        return StCas<float>((end - start) / 2.0f * (cnt * cnt * cnt * cnt * cnt + 2) + start);
    }

	// サイン波を利用したイージング
    float EasingInSine(float cnt, float start, float end, float frames)
    {
        return StCas<float>(-(end - start) * cos(cnt / frames * (PIOver2)) + end + start);
    }

	// サイン波を利用したイージング
    float EasingOutSine(float cnt, float start, float end, float frames)
    {
        return StCas<float>((end - start) * sin(cnt / frames * PIOver2) + start);
    }

	// サイン波を利用したイージング
    float EasingInOutSine(float cnt, float start, float end, float frames)
    {
        return StCas<float>(-(end - start) / 2.0 * (cos(PI * cnt / frames) - 1) + start);
    }

	// 指数関数を利用したイージング
    float EasingInExpo(float cnt, float start, float end, float frames)
    {
        return StCas<float>((end - start) * pow(2.0, 10 * (cnt / frames - 1)) + start);
    }

	// 指数関数を利用したイージング
    float EasingOutExpo(float cnt, float start, float end, float frames)
    {
        return StCas<float>((end - start) * (-pow(2.0, -10 * cnt / frames) + 1) + start);
    }

	// 指数関数を利用したイージング
    float EasingInOutExpo(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return StCas<float>((end - start) / 2.0 * pow(2.0, 10 * (cnt - 1)) + start);
        }
        cnt--;
        return StCas<float>((end - start) / 2.0 * (-pow(2.0, -10 * cnt) + 2) + start);
    }

	// 円を利用したイージング
    float EasingInCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        return -(end - start) * StCas<float>(sqrt(1 - cnt * cnt) - 1) + start;
    }

	// 円を利用したイージング
    float EasingOutCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames;
        cnt--;
        return StCas<float>((end - start) * sqrt(1 - cnt * cnt) + start);
    }

	// 円を利用したイージング
    float EasingInOutCirc(float cnt, float start, float end, float frames)
    {
        cnt /= frames / 2.0f;
        if(cnt < 1)
        {
            return StCas<float>(-(end - start) / 2.0 * (sqrt(1 - cnt * cnt) - 1) + start);
        }
        cnt -= 2;
        return StCas<float>((end - start) / 2.0 * (sqrt(1 - cnt * cnt) + 1) + start);
    }

	// ゴムのような弾性的なイージング(加速)
    float EasingInElastic(float cnt, float start, float end, float frames)
    {
        if(cnt == 0) return start; 
        cnt /= frames;
        if(cnt == 1) return end;
        float period = frames * 0.3f;  // 振動の周期
        float amplitude = end - start; // 振幅（振動の大きさ）
        float shift = period / 4.0f;   // 位相のシフト
        cnt--;
        return StCas<float>(-(amplitude * pow(2.0, 10 * cnt) * sin((cnt * frames - shift) * TwoPI / period)) + start); 
    }

    // バネのような弾性的なイージング（減速）
    float EasingOutElastic(float cnt, float start, float end, float frames)
    {
        if(cnt == 0) return start;
        cnt /= frames;
        if(cnt == 1) return end;
        float period = frames * 0.3f;
        float amplitude = end - start;
        float shift = period / 4.0f;
        return StCas<float>(amplitude * pow(2.0, -10 * cnt) * sin((cnt * frames - shift) * TwoPI / period) + end);
    }

    // バネのような弾性的なイージング（加速してから減速）
    float EasingInOutElastic(float cnt, float start, float end, float frames)
    {
        if(cnt == 0) return start;
        cnt /= frames / 2.0f;
        if(cnt == 2) return end;
        float period = frames * (0.3f * 1.5f);
        float amplitude = end - start;
        float shift = period / 4.0f;

        if(cnt < 1)
        {
            cnt--;
            return StCas<float>(-0.5f * (amplitude * pow(2.0, 10 * cnt) * sin((cnt * frames - shift) * TwoPI / period)) + start);
        }
        cnt--;
        return StCas<float>(amplitude * pow(2.0, -10 * cnt) * sin((cnt * frames - shift) * TwoPI / period) * 0.5f + end);
    }

    // ボールが跳ね返るようなイージング（加速）
    float EasingInBounce(float cnt, float start, float end, float frames)
    {
        return end - EasingOutBounce(frames - cnt, 0, end - start, frames);
    }

    // ボールが跳ね返るようなイージング（減速）
    float EasingOutBounce(float cnt, float start, float end, float frames)
    {
        float change = end - start;
        cnt /= frames;

        if(cnt < (1.0f / 2.75f))
        {
            return StCas<float>(change * (GRAVITY_COEFFICIENT * cnt * cnt) + start);
        }
        else if(cnt < (2.0f / 2.75f))
        {
            cnt -= (1.5f / 2.75f);
            return StCas<float>(change * (GRAVITY_COEFFICIENT * cnt * cnt + 0.75f) + start);
        }
        else if(cnt < (2.5f / 2.75f))
        {
            cnt -= (2.25f / 2.75f);
            return StCas<float>(change * (GRAVITY_COEFFICIENT * cnt * cnt + 0.9375f) + start);
        }
        else
        {
            cnt -= (2.625f / 2.75f);
            return StCas<float>(change * (GRAVITY_COEFFICIENT * cnt * cnt + 0.984375f) + start);
        }
    }

    // ボールが跳ね返るようなイージング（加速してから減速）
    float EasingInOutBounce(float cnt, float start, float end, float frames)
    {
        if(cnt < frames / 2.0f)
        {
            return StCas<float>(EasingInBounce(cnt * 2.0f, 0, end - start, frames) * 0.5f + start);
        }
        else
        {
            return StCas<float>(EasingOutBounce(cnt * 2.0f - frames, 0, end - start, frames) * 0.5f + (end - start) * 0.5f + start);
        }
    }

	// テスト用関数
    void TestEasing()
    {
		for(int i = 0; i <= 60; i++) // 0フレームから60フレームまでのイージング値をテスト表示
        {
            float val = EasingInOutQuad(static_cast<float>(i), 0.0f, 100.0f, 60.0f);
            printf("Frame %d: %f\n", i, val);
		}
    }
}