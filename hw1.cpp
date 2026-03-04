#include "hw1.h"
#include "hw1_scenes.h"

using namespace hw1;

Image3 hw_1_1(const std::vector<std::string> &params) {
    // Homework 1.1: render a circle at the specified
    // position, with the specified radius and color.

    Image3 img(640 /* width */, 480 /* height */);

    Vector2 center = Vector2{img.width / 2 + Real(0.5), img.height / 2 + Real(0.5)};
    Real radius = 100.0;
    Vector3 color = Vector3{1.0, 0.5, 0.5};
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-center") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            center = Vector2{x, y};
        } else if (params[i] == "-radius") {
            radius = std::stof(params[++i]);
        } else if (params[i] == "-color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            color = Vector3{r, g, b};
        }
    }
    // silence warnings, feel free to remove it
    UNUSED(radius);
    UNUSED(color);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector2 pixel_center = Vector2{x + Real(0.5), img.height - y - Real(0.5)};
            Vector2 pixel_vector = pixel_center - center;
            Real distance = length(pixel_vector);
            if (distance > radius) {
                img(x, y) = Vector3{0.5, 0.5, 0.5};
            }else{
                img(x, y) = color;
            }           
        }
    }
    return img;
}

// Helper function to compute distance from point to line segment
Real distance_to_segment(const Vector2& p, const Vector2& a, const Vector2& b) {
    Vector2 ab = b - a;
    Vector2 ap = p - a;

    Real ab_len2 = dot(ab, ab);
    if (ab_len2 == Real(0)) {
        // segment is a point
        return length(p - a);
    }

    Real t = dot(ap, ab) / ab_len2;
    // clamp only if caller needs full seg distance; we still return closest on segment
    t = std::clamp(t, Real(0), Real(1));
    Vector2 closest = a + t * ab;
    return length(p - closest);
}


// Helper function for point-in-polygon test using non-zero rule (winding number)
bool point_in_polygon(const Vector2& p, const std::vector<Vector2>& polygon) {
    int n = polygon.size();
    if (n < 3) return false;
    
    int winding_number = 0;
    
    for (int i = 0; i < n; i++) {
        Vector2 current = polygon[i];
        Vector2 next = polygon[(i + 1) % n];
        
        if (current.y <= p.y) {
            if (next.y > p.y) {
                // Upward crossing - check if point is to the left of the edge
                Real edge_cross = (next.x - current.x) * (p.y - current.y) - (p.x - current.x) * (next.y - current.y);
                if (edge_cross > 0) {
                    winding_number++;
                }
            }
        } else {
            if (next.y <= p.y) {
                // Downward crossing - check if point is to the right of the edge
                Real edge_cross = (next.x - current.x) * (p.y - current.y) - (p.x - current.x) * (next.y - current.y);
                if (edge_cross < 0) {
                    winding_number--;
                }
            }
        }
    }
    
    return winding_number != 0;
}

// Check if pixel_center is inside stroke region for polyline with flat caps & round joins.
// - flat caps: do NOT include endpoint semicircles for open polylines
// - round joins: add circular disks of radius half_width around internal vertices
bool is_in_stroke(const Vector2& pixel_center, const std::vector<Vector2>& polyline, 
                  bool is_closed, Real stroke_width) {
    if (polyline.size() < 2) return false;

    Real half_width = stroke_width / 2.0;

    // 1) Check all segments but only accept distance when projection t in [0,1].
    for (size_t i = 0; i + 1 < polyline.size(); ++i) {
        const Vector2 &a = polyline[i];
        const Vector2 &b = polyline[i + 1];
        Vector2 ab = b - a;
        Vector2 ap = pixel_center - a;

        Real ab_len2 = dot(ab, ab);
        if (ab_len2 == Real(0)) continue; // degenerate segment

        Real t = dot(ap, ab) / ab_len2;
        // only accept if projection hits the interior of the segment (flat caps)
        if (t >= Real(0) && t <= Real(1)) {
            Vector2 closest = a + t * ab;
            if (length(pixel_center - closest) <= half_width) return true;
        }
        // if t < 0 or t > 1 we DO NOT accept endpoint circle here (flat caps)
    }

    // 2) Closing segment if closed (same rule: t must be in [0,1] on that segment)
    if (is_closed && polyline.size() >= 3) {
        const Vector2 &a = polyline.back();
        const Vector2 &b = polyline.front();
        Vector2 ab = b - a;
        Vector2 ap = pixel_center - a;

        Real ab_len2 = dot(ab, ab);
        if (ab_len2 != Real(0)) {
            Real t = dot(ap, ab) / ab_len2;
            if (t >= Real(0) && t <= Real(1)) {
                Vector2 closest = a + t * ab;
                if (length(pixel_center - closest) <= half_width) return true;
            }
        }
    }

    // 3) Round joins: for closed polylines, every vertex is a join.
    // For open polylines, only interior vertices (1 .. n-2) get joins; ends do NOT get joins (flat caps).
    size_t n = polyline.size();
    if (is_closed) {
        for (size_t i = 0; i < n; ++i) {
            if (length(pixel_center - polyline[i]) <= half_width) return true;
        }
    } else {
        // interior vertices only
        for (size_t i = 1; i + 1 < n; ++i) {
            if (length(pixel_center - polyline[i]) <= half_width) return true;
        }
    }

    // Not in stroke
    return false;
}


Image3 hw_1_2(const std::vector<std::string> &params) {
    // Homework 1.2: render polylines
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Image3 img(640 /* width */, 480 /* height */);
    std::vector<Vector2> polyline;
    // is_closed = true indicates that the last point and
    // the first point of the polyline are connected
    bool is_closed = false;
    std::optional<Vector3> fill_color;
    std::optional<Vector3> stroke_color;
    Real stroke_width = 1;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-points") {
            while (params.size() > i+1 &&
                    params[i+1].length() > 0 &&
                    params[i+1][0] != '-') {
                Real x = std::stof(params[++i]);
                Real y = std::stof(params[++i]);
                polyline.push_back(Vector2{x, y});
            }
        } else if (params[i] == "--closed") {
            is_closed = true;
        } else if (params[i] == "-fill_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            fill_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            stroke_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_width") {
            stroke_width = std::stof(params[++i]);
        }
    }
    // silence warnings, feel free to remove it
    UNUSED(stroke_width);

    if (fill_color && !is_closed) {
        std::cout << "Error: can't have a non-closed shape with fill color." << std::endl;
        return Image3(0, 0);
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector2 pixel_center = Vector2{x + Real(0.5), img.height - y - Real(0.5)};
            Vector3 final_color  = Vector3{0.5, 0.5, 0.5};
            // Check if pixel should be filled
            if (is_closed && fill_color && polyline.size() >= 3) {
                if (point_in_polygon(pixel_center, polyline)) {
                    final_color = *fill_color;
                }
            }
            // Check if pixel should be stroked
            if (stroke_color && is_in_stroke(pixel_center, polyline, is_closed, stroke_width)) {
                final_color = *stroke_color;
            }
            
            img(x, y) = final_color;
        }
    }
    return img;
}

Image3 hw_1_3(const std::vector<std::string> &params) {
    // Homework 1.3: render multiple shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    // Initialize image to background
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            img(x, y) = scene.background;
        }
    }

    // Single pixel pass: evaluate shapes in scene order so later shapes overwrite earlier ones.
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Vector2 pixel_center = Vector2{ x + Real(0.5), img.height - y - Real(0.5) };
            Vector3 current_color = scene.background;

            for (auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
                const Shape &shape = *it;
                // Circle case
                if (auto *circle = std::get_if<Circle>(&shape)) {
                    Vector2 v = pixel_center - circle->center;
                    Real dist = length(v);

                    // fill
                    if (circle->fill_color && dist <= circle->radius) {
                        current_color = *circle->fill_color;
                    }

                    // stroke (stroke overwrites fill)
                    if (circle->stroke_color) {
                        Real half = circle->stroke_width / Real(2);
                        Real stroke_dist = std::abs(dist - circle->radius);
                        if (stroke_dist <= half) {
                            current_color = *circle->stroke_color;
                        }
                    }
                }
                // Polyline / polygon case
                else if (auto *polyline = std::get_if<Polyline>(&shape)) {
                    // fill (only for closed polylines with >= 3 points)
                    if (polyline->is_closed && polyline->fill_color && polyline->points.size() >= 3) {
                        if (point_in_polygon(pixel_center, polyline->points)) {
                            current_color = *polyline->fill_color;
                        }
                    }

                    // stroke (stroke overwrites fill)
                    if (polyline->stroke_color && polyline->points.size() >= 2) {
                        if (is_in_stroke(pixel_center, polyline->points, polyline->is_closed, polyline->stroke_width)) {
                            current_color = *polyline->stroke_color;
                        }
                    }
                }
                // if other shape types are added later, handle them here
            }

            img(x, y) = current_color;
        }
    }

    return img;
}

Image3 hw_1_4(const std::vector<std::string> &params) {
    // Homework 1.4: render transformed shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    // Initialize image to background
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            img(x, y) = scene.background;
        }
    }

    // For every pixel, map canvas-space pixel center to object-space using inverse transform
    // and evaluate untransformed primitives in object-space.
    for (int py = 0; py < img.height; ++py) {
        for (int px = 0; px < img.width; ++px) {
            Vector2 pixel_center = Vector2{ px + Real(0.5), img.height - py - Real(0.5) };
            Vector3 canvas_pt = Vector3{ pixel_center.x, pixel_center.y, Real(1.0) };

            Vector3 current_color = scene.background;

            // Iterate shapes in scene order (later shapes overwrite earlier)
            for (auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
                const Shape &shape = *it;
                // Circle
                if (auto *circle = std::get_if<Circle>(&shape)) {
                    // transform: convert canvas point to object space
                    Matrix3x3 F = circle->transform;            // object -> canvas
                    Matrix3x3 invF = inverse(F);                // canvas -> object
                    Vector3 obj3 = invF * canvas_pt;           // homogeneous
                    Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                    // Test fill in object space
                    Vector2 v = obj_pt - circle->center;
                    Real dist = length(v);
                    if (circle->fill_color && dist <= circle->radius) {
                        current_color = *circle->fill_color;
                    }

                    // Test stroke in object space (stroke overwrites fill)
                    if (circle->stroke_color) {
                        Real half = circle->stroke_width / Real(2);
                        Real stroke_dist = std::abs(dist - circle->radius);
                        if (stroke_dist <= half) {
                            current_color = *circle->stroke_color;
                        }
                    }
                }
                // Polyline / polygon
                else if (auto *polyline = std::get_if<Polyline>(&shape)) {
                    // map pixel center to object space using polyline transform
                    Matrix3x3 F = polyline->transform;
                    Matrix3x3 invF = inverse(F);
                    Vector3 obj3 = invF * canvas_pt;
                    Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                    // fill (only if closed and has fill color)
                    if (polyline->is_closed && polyline->fill_color && polyline->points.size() >= 3) {
                        if (point_in_polygon(obj_pt, polyline->points)) {
                            current_color = *polyline->fill_color;
                        }
                    }

                    // stroke (stroke overwrites fill)
                    if (polyline->stroke_color && polyline->points.size() >= 2) {
                        if (is_in_stroke(obj_pt, polyline->points, polyline->is_closed, polyline->stroke_width)) {
                            current_color = *polyline->stroke_color;
                        }
                    }
                }
            }

            img(px, py) = current_color;
        }
    }

    return img;
}


Image3 hw_1_5(const std::vector<std::string> &params) {
    // Homework 1.5: antialiasing (4x4 supersampling)
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    // Sub-sampling grid size (4x4)
    const int SS = 4;
    const int N_SAMPLES = SS * SS;
    const Real invSS = Real(1) / Real(SS);

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            // accumulator for RGB channels
            Real r_acc = Real(0), g_acc = Real(0), b_acc = Real(0);

            // iterate subpixel centers
            for (int sy = 0; sy < SS; ++sy) {
                for (int sx = 0; sx < SS; ++sx) {
                    // subpixel center inside pixel:
                    // sub_x = x + (sx + 0.5)/SS
                    // sub_y = y + (sy + 0.5)/SS
                    Real sub_x = Real(x) + (Real(sx) + Real(0.5)) * invSS;
                    Real sub_y = Real(y) + (Real(sy) + Real(0.5)) * invSS;

                    // convert to canvas coordinates (same convention used elsewhere)
                    Vector2 pixel_center = Vector2{ sub_x,
                        Real(img.height) - sub_y
                    }; // note: original code used img.height - (y + 0.5); here sub_y already includes the 0.5 offset

                    // homogeneous canvas point
                    Vector3 canvas_pt = Vector3{ pixel_center.x, pixel_center.y, Real(1.0) };

                    // start with background color for this sub-sample
                    Vector3 sample_color = scene.background;

                    // evaluate all shapes in scene order (later shapes overwrite earlier)
                    for (auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
                        const Shape &shape = *it;
                        // Circle
                        if (auto *circle = std::get_if<Circle>(&shape)) {
                            Matrix3x3 F = circle->transform;
                            Matrix3x3 invF = inverse(F);
                            Vector3 obj3 = invF * canvas_pt;
                            Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                            Vector2 v = obj_pt - circle->center;
                            Real dist = length(v);

                            // fill
                            if (circle->fill_color && dist <= circle->radius) {
                                sample_color = *circle->fill_color;
                            }

                            // stroke (overwrites fill)
                            if (circle->stroke_color) {
                                Real half = circle->stroke_width / Real(2);
                                Real stroke_dist = std::abs(dist - circle->radius);
                                if (stroke_dist <= half) {
                                    sample_color = *circle->stroke_color;
                                }
                            }
                        }
                        // Polyline / polygon
                        else if (auto *polyline = std::get_if<Polyline>(&shape)) {
                            Matrix3x3 F = polyline->transform;
                            Matrix3x3 invF = inverse(F);
                            Vector3 obj3 = invF * canvas_pt;
                            Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                            // fill (only for closed polylines with >= 3 points)
                            if (polyline->is_closed && polyline->fill_color && polyline->points.size() >= 3) {
                                if (point_in_polygon(obj_pt, polyline->points)) {
                                    sample_color = *polyline->fill_color;
                                }
                            }

                            // stroke (overwrites fill)
                            if (polyline->stroke_color && polyline->points.size() >= 2) {
                                if (is_in_stroke(obj_pt, polyline->points, polyline->is_closed, polyline->stroke_width)) {
                                    sample_color = *polyline->stroke_color;
                                }
                            }
                        }
                    } // end shapes loop

                    // accumulate sample_color
                    r_acc += sample_color.x;
                    g_acc += sample_color.y;
                    b_acc += sample_color.z;
                }
            }

            // average over all sub-samples
            Real inv_samples = Real(1) / Real(N_SAMPLES);
            Vector3 final_color = Vector3{ r_acc * inv_samples, g_acc * inv_samples, b_acc * inv_samples };

            img(x, y) = final_color;
        }
    }

    return img;
}


Image3 hw_1_6(const std::vector<std::string> &params) {
    // Homework 1.6: alpha blending (4x4 supersampling)
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    // Supersampling parameters: 4x4 grid
    const int SS = 4;
    const int N_SAMPLES = SS * SS;
    const Real invSS = Real(1) / Real(SS);
    const Real ONE = Real(1);

    for (int py = 0; py < img.height; ++py) {
        for (int px = 0; px < img.width; ++px) {
            // accumulator for sub-samples
            Vector3 accum = Vector3{Real(0), Real(0), Real(0)};

            for (int sy = 0; sy < SS; ++sy) {
                for (int sx = 0; sx < SS; ++sx) {
                    // compute subpixel center in image (canvas) coordinates
                    Real sub_x = Real(px) + (Real(sx) + Real(0.5)) * invSS;
                    Real sub_y = Real(py) + (Real(sy) + Real(0.5)) * invSS;

                    // convert to canvas-space point using your pixel-center conv (y-up)
                    Vector2 canvas_pt2 = Vector2{ sub_x, Real(img.height) - sub_y };
                    Vector3 canvas_pt = Vector3{ canvas_pt2.x, canvas_pt2.y, Real(1.0) };

                    // start with background (opaque)
                    Vector3 sample_color = scene.background;

                    // process shapes in scene order (later shapes are on top)
                    for (auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
                        const Shape &shape = *it;
                        // Circle
                        if (auto *circle = std::get_if<Circle>(&shape)) {
                            // map canvas -> object space
                            Matrix3x3 invF = inverse(circle->transform);
                            Vector3 obj3 = invF * canvas_pt;
                            Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                            // fill
                            if (circle->fill_color) {
                                Vector2 v = obj_pt - circle->center;
                                Real dist = length(v);
                                if (dist <= circle->radius) {
                                    Vector3 src = *circle->fill_color;
                                    Real a = circle->fill_alpha;
                                    // src over sample_color
                                    sample_color = a * src + (ONE - a) * sample_color;
                                }
                            }

                            // stroke (overwrites/comp composited over fill)
                            if (circle->stroke_color) {
                                Vector2 v = obj_pt - circle->center;
                                Real dist = length(v);
                                Real half = circle->stroke_width / Real(2);
                                Real stroke_dist = std::abs(dist - circle->radius);
                                if (stroke_dist <= half) {
                                    Vector3 src = *circle->stroke_color;
                                    Real a = circle->stroke_alpha;
                                    sample_color = a * src + (ONE - a) * sample_color;
                                }
                            }
                        }
                        // Polyline / polygon
                        else if (auto *polyline = std::get_if<Polyline>(&shape)) {
                            Matrix3x3 invF = inverse(polyline->transform);
                            Vector3 obj3 = invF * canvas_pt;
                            Vector2 obj_pt = Vector2{ obj3.x, obj3.y };

                            // fill (only when closed and has fill_color)
                            if (polyline->is_closed && polyline->fill_color && polyline->points.size() >= 3) {
                                if (point_in_polygon(obj_pt, polyline->points)) {
                                    Vector3 src = *polyline->fill_color;
                                    Real a = polyline->fill_alpha;
                                    sample_color = a * src + (ONE - a) * sample_color;
                                }
                            }

                            // stroke
                            if (polyline->stroke_color && polyline->points.size() >= 2) {
                                if (is_in_stroke(obj_pt, polyline->points, polyline->is_closed, polyline->stroke_width)) {
                                    Vector3 src = *polyline->stroke_color;
                                    Real a = polyline->stroke_alpha;
                                    sample_color = a * src + (ONE - a) * sample_color;
                                }
                            }
                        }
                    } // end shapes loop

                    accum = accum + sample_color;
                }
            }

            // average samples
            Vector3 final_color = (Real(1) / Real(N_SAMPLES)) * accum;
            img(px, py) = final_color;
        }
    }

    return img;
}

