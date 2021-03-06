#include "akane/render.h"
#include "akane/integrator/path_tracing.h"

namespace akane
{
    RenderResult ExecuteRenderingSingleThread(
        const Integrator& integrator, const Scene& scene, const Camera& camera, Point2i resolution,
        int sample_per_pixel, int thread_id, unsigned seed, std::function<bool()>* activity_query,
        std::function<void(int, const RenderResult&)>* checkpoint_handler)
    {
        RenderResult result{};
        result.canvas = make_shared<Canvas>(resolution[0], resolution[1]);

        RenderingContext ctx;
        auto working_canvas = std::make_shared<Canvas>(resolution[0], resolution[1]);

        auto sampler = CreateRandomSampler(seed);

        int ssp_per_batch =
            static_cast<int>(ceil(sample_per_pixel / min(sample_per_pixel * 1.f, 100.f)));

        int progress = 0;
        for (int batch = 0; batch < 100; ++batch)
        {
            auto ssp_this_batch = min(ssp_per_batch, sample_per_pixel - result.ssp);
            if (ssp_this_batch <= 0)
            {
                break;
            }

            bool active = true;
            for (int y = 0; active && y < resolution[1]; ++y)
            {
                for (int x = 0; active && x < resolution[0]; ++x)
                {
                    Spectrum radiance = 0.f;
                    for (int i = 0; i < ssp_this_batch; ++i)
                    {
                        auto uv  = ComputeScreenSpaceUV({x, y}, resolution, sampler->Get2D());
                        auto ray = camera.SpawnRay(uv);

                        radiance += integrator.Li(ctx, *sampler, scene, ray);
                    }

                    working_canvas->IncrementPixel(x, y, radiance);
                }

                // query if rendering should continue
                if (activity_query != nullptr && !(*activity_query)())
                {
                    active = false;
                }
            }

            if (active)
            {
                result.ssp += ssp_this_batch;
                result.canvas->Set(*working_canvas);

                progress = static_cast<int>(result.ssp * 100.f / sample_per_pixel);
            }
            else
            {
                break;
            }

            if (checkpoint_handler != nullptr)
            {
                (*checkpoint_handler)(progress, result);
            }
        }

        return result;
    }

    RenderResult
    ExecuteRenderingMultiThread(const Integrator& integrator, const Scene& scene,
                                const Camera& camera, Point2i resolution, int sample_per_pixel,
                                int thread_count, std::function<bool()>* activity_query,
                                std::function<void(int, const RenderResult&)>* checkpoint_handler)
    {
        auto execute_render = [&](unsigned seed, int ssp, int thd_id) {
            fmt::print("[thd {}] assigned with {} ssp, start working...\n", thd_id, ssp);

            std::function<void(int, const RenderResult&)> checkpoint_forward_handler =
                [&](int progress, const RenderResult& checkpoint) {
                    fmt::print("[thd {}] {} ssp finished({}%)\n", thd_id, checkpoint.ssp, progress);

                    if (checkpoint_handler != nullptr)
                    {
                        (*checkpoint_handler)(progress, checkpoint);
                    }
                };

            auto result =
                ExecuteRenderingSingleThread(integrator, scene, camera, resolution, ssp, thd_id,
                                             seed, activity_query, &checkpoint_forward_handler);
            fmt::print("[thd {}] tasks done, exiting...\n", thd_id, result.ssp);

            return result;
        };

        std::random_device rnd{};
        if (thread_count == 1)
        {
            return execute_render(rnd(), sample_per_pixel, 0);
        }
        else
        {
            std::vector<std::future<RenderResult>> futures;
            for (int i = 0; i < thread_count; ++i)
            {
                auto ssp =
                    sample_per_pixel / thread_count + (i < sample_per_pixel % thread_count ? 1 : 0);
                futures.push_back(std::async(execute_render, rnd(), ssp, i));
            }

            auto ssp    = 0;
            auto canvas = std::make_shared<Canvas>(resolution[0], resolution[1]);
            for (auto& future : futures)
            {
                future.wait();
                auto result = future.get();

                ssp += result.ssp;
                canvas->Increment(*result.canvas);
            }

            return RenderResult{ssp, canvas};
        }
    }
} // namespace akane