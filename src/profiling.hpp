#pragma once

namespace kovra {
struct RendererStats
{
    float frame_time;
    int triangle_count;
    int draw_call_count;
    float scene_update_time;
    float render_objects_draw_time;
};
}
