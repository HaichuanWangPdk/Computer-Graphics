#include "hw2.h"
#include "hw2_scenes.h"

using namespace hw2;

Image3 hw_2_1(const std::vector<std::string> &params) {
    // Homework 2.1: render a single 3D triangle

    Image3 img(640 /* width */, 480 /* height */);

    Vector3 p0{0, 0, -1};
    Vector3 p1{1, 0, -1};
    Vector3 p2{0, 1, -1};
    Real s = 1; // scaling factor of the view frustrum
    Vector3 color = Vector3{1.0, 0.5, 0.5};
    Real z_near = 1e-6; // distance of the near clipping plane
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-p0") {
            p0.x = std::stof(params[++i]);
            p0.y = std::stof(params[++i]);
            p0.z = std::stof(params[++i]);
        } else if (params[i] == "-p1") {
            p1.x = std::stof(params[++i]);
            p1.y = std::stof(params[++i]);
            p1.z = std::stof(params[++i]);
        } else if (params[i] == "-p2") {
            p2.x = std::stof(params[++i]);
            p2.y = std::stof(params[++i]);
            p2.z = std::stof(params[++i]);
        } else if (params[i] == "-color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            color = Vector3{r, g, b};
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        }
    }

    // Check if all vertices are in front of the near clipping plane
    // Only reject triangle if ANY vertex is behind near plane
    if (p0.z > -z_near || p1.z > -z_near || p2.z > -z_near) {
        // If any vertex is behind near plane, render gray background
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img(x, y) = Vector3{0.5, 0.5, 0.5};
            }
        }
        return img;
    }

    // Project 3D points to 2D image plane using perspective projection
    // We project ALL vertices, even if they're outside view frustrum
    auto project_point = [&](const Vector3& p) -> Vector2 {
        // Perspective projection: (x, y, z) -> (x/(-z), y/(-z))
        Real x_proj = p.x / (-p.z);
        Real y_proj = p.y / (-p.z);
        
        // Convert from projected camera space to screen space
        Real aspect_ratio = Real(img.width) / Real(img.height);
        Real x_min = -s * aspect_ratio;
        Real x_max = s * aspect_ratio;
        Real y_min = -s;
        Real y_max = s;
        
        // Map from [-s*a, s*a] to [0, width] and [-s, s] to [0, height] (flip y)
        Real x_screen = (x_proj - x_min) / (x_max - x_min) * img.width;
        Real y_screen = (1.0 - (y_proj - y_min) / (y_max - y_min)) * img.height;
        
        return Vector2{x_screen, y_screen};
    };

    // Project all three vertices (we don't reject for being outside view frustrum)
    Vector2 v0 = project_point(p0);
    Vector2 v1 = project_point(p1);
    Vector2 v2 = project_point(p2);

    // Helper function for edge function (determines which side of edge point is on)
    auto edge_function = [](const Vector2& a, const Vector2& b, const Vector2& p) -> Real {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    // 4x4 supersampling for antialiasing
    const int samples_per_pixel = 4;
    const Real subpixel_size = Real(1) / samples_per_pixel;
    const Real subpixel_offset = subpixel_size / Real(2);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated_color = Vector3{0, 0, 0};
            
            // Sample 4x4 grid within the pixel
            for (int sy = 0; sy < samples_per_pixel; sy++) {
                for (int sx = 0; sx < samples_per_pixel; sx++) {
                    // Calculate subpixel center position
                    Real subpixel_x = x + sx * subpixel_size + subpixel_offset;
                    Real subpixel_y = y + sy * subpixel_size + subpixel_offset;
                    
                    Vector2 p = Vector2{subpixel_x, subpixel_y};
                    
                    // Compute edge functions
                    Real w0 = edge_function(v1, v2, p);
                    Real w1 = edge_function(v2, v0, p);
                    Real w2 = edge_function(v0, v1, p);
                    
                    // Check if point is inside triangle (all edge functions have same sign)
                    if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                        accumulated_color = accumulated_color + color;
                    } else {
                        accumulated_color = accumulated_color + Vector3{0.5, 0.5, 0.5};
                    }
                }
            }
            
            // Average the samples
            img(x, y) = accumulated_color / Real(samples_per_pixel * samples_per_pixel);
        }
    }

    return img;
}

Image3 hw_2_2(const std::vector<std::string> &params) {
    // Homework 2.2: render a triangle mesh

    Image3 img(640 /* width */, 480 /* height */);

    Real s = 1; // scaling factor of the view frustrum
    Real z_near = 1e-6; // distance of the near clipping plane
    int scene_id = 0;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        } else if (params[i] == "-scene_id") {
            scene_id = std::stoi(params[++i]);
        }
    }

    TriangleMesh mesh = meshes[scene_id];

    // 4x4 supersampling for antialiasing
    const int samples_per_pixel = 4;
    const Real subpixel_size = Real(1) / samples_per_pixel;
    const Real subpixel_offset = subpixel_size / Real(2);
    
    // Create supersampled buffers
    int super_width = img.width * samples_per_pixel;
    int super_height = img.height * samples_per_pixel;
    std::vector<Vector3> super_color(super_width * super_height, Vector3{0.5, 0.5, 0.5});
    
    // Initialize Z-buffer to CLOSEST depth (most negative value)
    std::vector<Real> super_depth(super_width * super_height, -std::numeric_limits<Real>::max());

    // Helper function to project 3D point to 2D screen space
    auto project_point = [&img, s](const Vector3& p) -> Vector2 {
        // Perspective projection: (x, y, z) -> (x/(-z), y/(-z))
        Real x_proj = p.x / (-p.z);
        Real y_proj = p.y / (-p.z);
        
        // Convert from projected camera space to screen space
        Real aspect_ratio = Real(img.width) / Real(img.height);
        Real x_min = -s * aspect_ratio;
        Real x_max = s * aspect_ratio;
        Real y_min = -s;
        Real y_max = s;
        
        // Map from [-s*a, s*a] to [0, width] and [-s, s] to [0, height] (flip y)
        Real x_screen = (x_proj - x_min) / (x_max - x_min) * img.width;
        Real y_screen = (1.0 - (y_proj - y_min) / (y_max - y_min)) * img.height;
        
        return Vector2{x_screen, y_screen};
    };

    // Helper function for edge function (2D)
    auto edge_function = [](const Vector2& a, const Vector2& b, const Vector2& p) -> Real {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    // Helper function to compute triangle area (2D)
    auto triangle_area = [edge_function](const Vector2& a, const Vector2& b, const Vector2& c) -> Real {
        return std::abs(edge_function(a, b, c)) / 2;
    };

    // Render each triangle in the mesh
    for (int face_idx = 0; face_idx < (int)mesh.faces.size(); face_idx++) {
        const Vector3i& face = mesh.faces[face_idx];
        Vector3 color = mesh.face_colors[face_idx];
        
        // Get triangle vertices
        Vector3 p0 = mesh.vertices[face.x];
        Vector3 p1 = mesh.vertices[face.y];
        Vector3 p2 = mesh.vertices[face.z];
        
        // Check if all vertices are in front of near plane
        // Camera looks along negative z, so we want z < -z_near
        if (p0.z >= -z_near || p1.z >= -z_near || p2.z >= -z_near) {
            continue; // Skip triangle if any vertex is behind near plane
        }
        
        // Project vertices to screen space
        Vector2 v0 = project_point(p0);
        Vector2 v1 = project_point(p1);
        Vector2 v2 = project_point(p2);
        
        // Compute bounding box of projected triangle (in supersampled coordinates)
        int min_x = std::max(0, (int)std::floor(std::min({v0.x, v1.x, v2.x}) * samples_per_pixel));
        int max_x = std::min(super_width - 1, (int)std::ceil(std::max({v0.x, v1.x, v2.x}) * samples_per_pixel));
        int min_y = std::max(0, (int)std::floor(std::min({v0.y, v1.y, v2.y}) * samples_per_pixel));
        int max_y = std::min(super_height - 1, (int)std::ceil(std::max({v0.y, v1.y, v2.y}) * samples_per_pixel));
        
        if (min_x > max_x || min_y > max_y) continue;
        
        // Precompute areas for barycentric coordinates
        Real area_abc = triangle_area(v0, v1, v2);
        if (area_abc < 1e-9) continue; // Skip degenerate triangles
        
        // Precompute edge functions for the entire triangle
        Real edge0_base = edge_function(v1, v2, v0); // Actually the area * 2
        Real edge1_base = edge_function(v2, v0, v1);
        Real edge2_base = edge_function(v0, v1, v2);
        
        // Determine winding order to get correct sign for inside test
        bool clockwise = edge0_base < 0;
        
        // Rasterize triangle using bounding box optimization
        for (int sy = min_y; sy <= max_y; sy++) {
            for (int sx = min_x; sx <= max_x; sx++) {
                // Convert supersampled coordinates back to screen space
                Vector2 p_screen = Vector2{
                    Real(sx) / samples_per_pixel + subpixel_offset,
                    Real(sy) / samples_per_pixel + subpixel_offset
                };
                
                // Compute edge functions for current point
                Real w0 = edge_function(v1, v2, p_screen);
                Real w1 = edge_function(v2, v0, p_screen);
                Real w2 = edge_function(v0, v1, p_screen);
                
                // Check if point is inside triangle with robust epsilon
                // Use the same sign as the base edges to handle both winding orders
                bool inside = false;
                if (clockwise) {
                    inside = (w0 <= 1e-5) && (w1 <= 1e-5) && (w2 <= 1e-5);
                } else {
                    inside = (w0 >= -1e-5) && (w1 >= -1e-5) && (w2 >= -1e-5);
                }
                
                if (inside) {
                    // Convert to barycentric coordinates
                    Real b0 = w0 / edge0_base;
                    Real b1 = w1 / edge1_base;
                    Real b2 = w2 / edge2_base;
                    
                    // Normalize barycentric coordinates (for robustness)
                    Real sum = b0 + b1 + b2;
                    if (std::abs(sum - 1.0) > 1e-5) {
                        b0 /= sum;
                        b1 /= sum;
                        b2 /= sum;
                    }
                    
                    // Convert screen-space barycentric to 3D barycentric using perspective correction
                    Real w0_3d = b0 / (-p0.z);
                    Real w1_3d = b1 / (-p1.z);
                    Real w2_3d = b2 / (-p2.z);
                    Real inv_sum = 1.0 / (w0_3d + w1_3d + w2_3d);
                    
                    Real b0_3d = w0_3d * inv_sum;
                    Real b1_3d = w1_3d * inv_sum;
                    Real b2_3d = w2_3d * inv_sum;
                    
                    // Compute interpolated depth (z coordinate)
                    Real depth = b0_3d * p0.z + b1_3d * p1.z + b2_3d * p2.z;
                    
                    // Update pixel if this triangle is CLOSER (more negative z = closer to camera)
                    int super_idx = sy * super_width + sx;
                    if (depth > super_depth[super_idx]) {  // CHANGED: > instead of <
                        super_depth[super_idx] = depth;
                        super_color[super_idx] = color;
                    }
                }
            }
        }
    }
    
    // Resolve supersampled image to final image
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated_color = Vector3{0, 0, 0};
            
            // Average 4x4 samples
            for (int sy = 0; sy < samples_per_pixel; sy++) {
                for (int sx = 0; sx < samples_per_pixel; sx++) {
                    int super_x = x * samples_per_pixel + sx;
                    int super_y = y * samples_per_pixel + sy;
                    accumulated_color = accumulated_color + super_color[super_y * super_width + super_x];
                }
            }
            
            img(x, y) = accumulated_color / Real(samples_per_pixel * samples_per_pixel);
        }
    }

    return img;
}

Image3 hw_2_3(const std::vector<std::string> &params) {
    // Homework 2.3: render a triangle mesh with vertex colors

    Image3 img(640 /* width */, 480 /* height */);

    Real s = 1; // scaling factor of the view frustrum
    Real z_near = 1e-6; // distance of the near clipping plane
    int scene_id = 0;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        } else if (params[i] == "-scene_id") {
            scene_id = std::stoi(params[++i]);
        }
    }

    TriangleMesh mesh = meshes[scene_id];

    // 4x4 supersampling for antialiasing
    const int samples_per_pixel = 4;
    const Real subpixel_size = Real(1) / samples_per_pixel;
    const Real subpixel_offset = subpixel_size / Real(2);
    
    // Create supersampled buffers
    int super_width = img.width * samples_per_pixel;
    int super_height = img.height * samples_per_pixel;
    std::vector<Vector3> super_color(super_width * super_height, Vector3{0.5, 0.5, 0.5});
    
    // Initialize Z-buffer to CLOSEST depth (most negative value)
    std::vector<Real> super_depth(super_width * super_height, -std::numeric_limits<Real>::max());

    // Helper function to project 3D point to 2D screen space
    auto project_point = [&img, s](const Vector3& p) -> Vector2 {
        // Perspective projection: (x, y, z) -> (x/(-z), y/(-z))
        Real x_proj = p.x / (-p.z);
        Real y_proj = p.y / (-p.z);
        
        // Convert from projected camera space to screen space
        Real aspect_ratio = Real(img.width) / Real(img.height);
        Real x_min = -s * aspect_ratio;
        Real x_max = s * aspect_ratio;
        Real y_min = -s;
        Real y_max = s;
        
        // Map from [-s*a, s*a] to [0, width] and [-s, s] to [0, height] (flip y)
        Real x_screen = (x_proj - x_min) / (x_max - x_min) * img.width;
        Real y_screen = (1.0 - (y_proj - y_min) / (y_max - y_min)) * img.height;
        
        return Vector2{x_screen, y_screen};
    };

    // Helper function for edge function (2D)
    auto edge_function = [](const Vector2& a, const Vector2& b, const Vector2& p) -> Real {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    // Helper function to compute triangle area (2D)
    auto triangle_area = [edge_function](const Vector2& a, const Vector2& b, const Vector2& c) -> Real {
        return std::abs(edge_function(a, b, c)) / 2;
    };

    // Render each triangle in the mesh
    for (int face_idx = 0; face_idx < (int)mesh.faces.size(); face_idx++) {
        const Vector3i& face = mesh.faces[face_idx];
        
        // Get triangle vertices and their colors
        Vector3 p0 = mesh.vertices[face.x];
        Vector3 p1 = mesh.vertices[face.y];
        Vector3 p2 = mesh.vertices[face.z];
        Vector3 c0 = mesh.vertex_colors[face.x];
        Vector3 c1 = mesh.vertex_colors[face.y];
        Vector3 c2 = mesh.vertex_colors[face.z];
        
        // Check if all vertices are in front of near plane
        // Camera looks along negative z, so we want z < -z_near
        if (p0.z >= -z_near || p1.z >= -z_near || p2.z >= -z_near) {
            continue; // Skip triangle if any vertex is behind near plane
        }
        
        // Project vertices to screen space
        Vector2 v0 = project_point(p0);
        Vector2 v1 = project_point(p1);
        Vector2 v2 = project_point(p2);
        
        // Compute bounding box of projected triangle (in supersampled coordinates)
        int min_x = std::max(0, (int)std::floor(std::min({v0.x, v1.x, v2.x}) * samples_per_pixel));
        int max_x = std::min(super_width - 1, (int)std::ceil(std::max({v0.x, v1.x, v2.x}) * samples_per_pixel));
        int min_y = std::max(0, (int)std::floor(std::min({v0.y, v1.y, v2.y}) * samples_per_pixel));
        int max_y = std::min(super_height - 1, (int)std::ceil(std::max({v0.y, v1.y, v2.y}) * samples_per_pixel));
        
        if (min_x > max_x || min_y > max_y) continue;
        
        // Precompute areas for barycentric coordinates
        Real area_abc = triangle_area(v0, v1, v2);
        if (area_abc < 1e-9) continue; // Skip degenerate triangles
        
        // Precompute edge functions for the entire triangle
        Real edge0_base = edge_function(v1, v2, v0); // Actually the area * 2
        Real edge1_base = edge_function(v2, v0, v1);
        Real edge2_base = edge_function(v0, v1, v2);
        
        // Determine winding order to get correct sign for inside test
        bool clockwise = edge0_base < 0;
        
        // Rasterize triangle using bounding box optimization
        for (int sy = min_y; sy <= max_y; sy++) {
            for (int sx = min_x; sx <= max_x; sx++) {
                // Convert supersampled coordinates back to screen space
                Vector2 p_screen = Vector2{
                    Real(sx) / samples_per_pixel + subpixel_offset,
                    Real(sy) / samples_per_pixel + subpixel_offset
                };
                
                // Compute edge functions for current point
                Real w0 = edge_function(v1, v2, p_screen);
                Real w1 = edge_function(v2, v0, p_screen);
                Real w2 = edge_function(v0, v1, p_screen);
                
                // Check if point is inside triangle with robust epsilon
                // Use the same sign as the base edges to handle both winding orders
                bool inside = false;
                if (clockwise) {
                    inside = (w0 <= 1e-5) && (w1 <= 1e-5) && (w2 <= 1e-5);
                } else {
                    inside = (w0 >= -1e-5) && (w1 >= -1e-5) && (w2 >= -1e-5);
                }
                
                if (inside) {
                    // Convert to barycentric coordinates in screen space
                    Real b0_screen = w0 / edge0_base;
                    Real b1_screen = w1 / edge1_base;
                    Real b2_screen = w2 / edge2_base;
                    
                    // Normalize barycentric coordinates (for robustness)
                    Real sum_screen = b0_screen + b1_screen + b2_screen;
                    if (std::abs(sum_screen - 1.0) > 1e-5) {
                        b0_screen /= sum_screen;
                        b1_screen /= sum_screen;
                        b2_screen /= sum_screen;
                    }
                    
                    // Convert screen-space barycentric to 3D barycentric using perspective correction
                    // This is the key step for perspective-correct interpolation
                    Real w0_3d = b0_screen / (-p0.z);
                    Real w1_3d = b1_screen / (-p1.z);
                    Real w2_3d = b2_screen / (-p2.z);
                    Real inv_sum_3d = 1.0 / (w0_3d + w1_3d + w2_3d);
                    
                    Real b0_3d = w0_3d * inv_sum_3d;
                    Real b1_3d = w1_3d * inv_sum_3d;
                    Real b2_3d = w2_3d * inv_sum_3d;
                    
                    // Compute interpolated depth (z coordinate)
                    Real depth = b0_3d * p0.z + b1_3d * p1.z + b2_3d * p2.z;
                    
                    // Interpolate vertex colors using perspective-correct barycentric coordinates
                    Vector3 color = b0_3d * c0 + b1_3d * c1 + b2_3d * c2;
                    
                    // Update pixel if this triangle is CLOSER (more negative z = closer to camera)
                    int super_idx = sy * super_width + sx;
                    if (depth > super_depth[super_idx]) {
                        super_depth[super_idx] = depth;
                        super_color[super_idx] = color;
                    }
                }
            }
        }
    }
    
    // Resolve supersampled image to final image
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated_color = Vector3{0, 0, 0};
            
            // Average 4x4 samples
            for (int sy = 0; sy < samples_per_pixel; sy++) {
                for (int sx = 0; sx < samples_per_pixel; sx++) {
                    int super_x = x * samples_per_pixel + sx;
                    int super_y = y * samples_per_pixel + sy;
                    accumulated_color = accumulated_color + super_color[super_y * super_width + super_x];
                }
            }
            
            img(x, y) = accumulated_color / Real(samples_per_pixel * samples_per_pixel);
        }
    }

    return img;
}

Image3 hw_2_4(const std::vector<std::string> &params) {
    // Homework 2.4: render a scene with transformation
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.camera.resolution.x,
               scene.camera.resolution.y);

    // 4x4 supersampling for antialiasing
    const int samples_per_pixel = 4;
    const Real subpixel_size = Real(1) / samples_per_pixel;
    const Real subpixel_offset = subpixel_size / Real(2);
    
    // Create supersampled buffers
    int super_width = img.width * samples_per_pixel;
    int super_height = img.height * samples_per_pixel;
    std::vector<Vector3> super_color(super_width * super_height, scene.background);
    
    // Initialize Z-buffer to CLOSEST depth (most negative value)
    std::vector<Real> super_depth(super_width * super_height, -std::numeric_limits<Real>::max());

    // Compute view matrix (world to camera)
    Matrix4x4 view_matrix = inverse(scene.camera.cam_to_world);

    // Helper function to transform and project a 3D point to 2D screen space
    auto transform_and_project_point = [&](const Vector3& p, const Matrix4x4& model_matrix) -> Vector3 {
        // Apply model-view transformation: world -> camera space
        Vector4 p_camera = view_matrix * model_matrix * Vector4{p.x, p.y, p.z, 1.0};
        
        // Perspective projection in camera space
        if (p_camera.z >= -scene.camera.z_near) {
            // Point is behind near plane, return invalid point
            return Vector3{0, 0, 0};
        }
        
        Real x_proj = p_camera.x / (-p_camera.z);
        Real y_proj = p_camera.y / (-p_camera.z);
        
        // Convert from projected camera space to screen space
        Real aspect_ratio = Real(img.width) / Real(img.height);
        Real x_min = -scene.camera.s * aspect_ratio;
        Real x_max = scene.camera.s * aspect_ratio;
        Real y_min = -scene.camera.s;
        Real y_max = scene.camera.s;
        
        // Map from [-s*a, s*a] to [0, width] and [-s, s] to [0, height] (flip y)
        Real x_screen = (x_proj - x_min) / (x_max - x_min) * img.width;
        Real y_screen = (1.0 - (y_proj - y_min) / (y_max - y_min)) * img.height;
        
        return Vector3{x_screen, y_screen, p_camera.z};
    };

    // Helper function for edge function (2D)
    auto edge_function = [](const Vector2& a, const Vector2& b, const Vector2& p) -> Real {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    // Helper function to compute triangle area (2D)
    auto triangle_area = [edge_function](const Vector2& a, const Vector2& b, const Vector2& c) -> Real {
        return std::abs(edge_function(a, b, c)) / 2;
    };

    // Render each mesh in the scene
    for (const auto& mesh : scene.meshes) {
        Matrix4x4 model_matrix = mesh.model_matrix;
        
        // Render each triangle in the mesh
        for (int face_idx = 0; face_idx < (int)mesh.faces.size(); face_idx++) {
            const Vector3i& face = mesh.faces[face_idx];
            
            // Get triangle vertices and their colors
            Vector3 p0 = mesh.vertices[face.x];
            Vector3 p1 = mesh.vertices[face.y];
            Vector3 p2 = mesh.vertices[face.z];
            Vector3 c0 = mesh.vertex_colors[face.x];
            Vector3 c1 = mesh.vertex_colors[face.y];
            Vector3 c2 = mesh.vertex_colors[face.z];
            
            // Transform vertices to camera space and project to screen space
            Vector3 v0_proj = transform_and_project_point(p0, model_matrix);
            Vector3 v1_proj = transform_and_project_point(p1, model_matrix);
            Vector3 v2_proj = transform_and_project_point(p2, model_matrix);
            
            // Skip triangle if any vertex is behind near plane
            if (v0_proj.z == 0 || v1_proj.z == 0 || v2_proj.z == 0) {
                continue;
            }
            
            Vector2 v0 = Vector2{v0_proj.x, v0_proj.y};
            Vector2 v1 = Vector2{v1_proj.x, v1_proj.y};
            Vector2 v2 = Vector2{v2_proj.x, v2_proj.y};
            
            // Store original z values for perspective correction
            Real z0 = v0_proj.z;
            Real z1 = v1_proj.z;
            Real z2 = v2_proj.z;
            
            // Compute bounding box of projected triangle (in supersampled coordinates)
            int min_x = std::max(0, (int)std::floor(std::min({v0.x, v1.x, v2.x}) * samples_per_pixel));
            int max_x = std::min(super_width - 1, (int)std::ceil(std::max({v0.x, v1.x, v2.x}) * samples_per_pixel));
            int min_y = std::max(0, (int)std::floor(std::min({v0.y, v1.y, v2.y}) * samples_per_pixel));
            int max_y = std::min(super_height - 1, (int)std::ceil(std::max({v0.y, v1.y, v2.y}) * samples_per_pixel));
            
            if (min_x > max_x || min_y > max_y) continue;
            
            // Precompute areas for barycentric coordinates
            Real area_abc = triangle_area(v0, v1, v2);
            if (area_abc < 1e-9) continue; // Skip degenerate triangles
            
            // Precompute edge functions for the entire triangle
            Real edge0_base = edge_function(v1, v2, v0);
            Real edge1_base = edge_function(v2, v0, v1);
            Real edge2_base = edge_function(v0, v1, v2);
            
            // Determine winding order to get correct sign for inside test
            bool clockwise = edge0_base < 0;
            
            // Rasterize triangle using bounding box optimization
            for (int sy = min_y; sy <= max_y; sy++) {
                for (int sx = min_x; sx <= max_x; sx++) {
                    // Convert supersampled coordinates back to screen space
                    Vector2 p_screen = Vector2{
                        Real(sx) / samples_per_pixel + subpixel_offset,
                        Real(sy) / samples_per_pixel + subpixel_offset
                    };
                    
                    // Compute edge functions for current point
                    Real w0 = edge_function(v1, v2, p_screen);
                    Real w1 = edge_function(v2, v0, p_screen);
                    Real w2 = edge_function(v0, v1, p_screen);
                    
                    // Check if point is inside triangle with robust epsilon
                    bool inside = false;
                    if (clockwise) {
                        inside = (w0 <= 1e-5) && (w1 <= 1e-5) && (w2 <= 1e-5);
                    } else {
                        inside = (w0 >= -1e-5) && (w1 >= -1e-5) && (w2 >= -1e-5);
                    }
                    
                    if (inside) {
                        // Convert to barycentric coordinates in screen space
                        Real b0_screen = w0 / edge0_base;
                        Real b1_screen = w1 / edge1_base;
                        Real b2_screen = w2 / edge2_base;
                        
                        // Normalize barycentric coordinates (for robustness)
                        Real sum_screen = b0_screen + b1_screen + b2_screen;
                        if (std::abs(sum_screen - 1.0) > 1e-5) {
                            b0_screen /= sum_screen;
                            b1_screen /= sum_screen;
                            b2_screen /= sum_screen;
                        }
                        
                        // Convert screen-space barycentric to 3D barycentric using perspective correction
                        Real w0_3d = b0_screen / (-z0);
                        Real w1_3d = b1_screen / (-z1);
                        Real w2_3d = b2_screen / (-z2);
                        Real inv_sum_3d = 1.0 / (w0_3d + w1_3d + w2_3d);
                        
                        Real b0_3d = w0_3d * inv_sum_3d;
                        Real b1_3d = w1_3d * inv_sum_3d;
                        Real b2_3d = w2_3d * inv_sum_3d;
                        
                        // Compute interpolated depth (z coordinate)
                        Real depth = b0_3d * z0 + b1_3d * z1 + b2_3d * z2;
                        
                        // Interpolate vertex colors using perspective-correct barycentric coordinates
                        Vector3 color = b0_3d * c0 + b1_3d * c1 + b2_3d * c2;
                        
                        // Update pixel if this triangle is CLOSER (more negative z = closer to camera)
                        int super_idx = sy * super_width + sx;
                        if (depth > super_depth[super_idx]) {
                            super_depth[super_idx] = depth;
                            super_color[super_idx] = color;
                        }
                    }
                }
            }
        }
    }
    
    // Resolve supersampled image to final image
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated_color = Vector3{0, 0, 0};
            
            // Average 4x4 samples
            for (int sy = 0; sy < samples_per_pixel; sy++) {
                for (int sx = 0; sx < samples_per_pixel; sx++) {
                    int super_x = x * samples_per_pixel + sx;
                    int super_y = y * samples_per_pixel + sy;
                    accumulated_color = accumulated_color + super_color[super_y * super_width + super_x];
                }
            }
            
            img(x, y) = accumulated_color / Real(samples_per_pixel * samples_per_pixel);
        }
    }

    return img;
}

