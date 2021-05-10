import header
precision highp uimage2D;

layout(local_size_x = 32, local_size_y = 32) in;
layout(r32ui, binding = 0) uniform writeonly uimage2D output_tex;

uniform sampler2D depth_tex;
uniform mat4 proj_view_inversed;

layout(binding = 3) buffer pe_lights
{
	vec4 lights[];
};

compute:
	vec3 pixel_pos(vec3 clip)
	{
		vec4 p = proj_view_inversed * vec4(clip * 2.0 - 1.0, 1.0);
		return p.xyz / p.w;
	}

	void main()
	{
		ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

		if(coord.x >= VIEWPORT_SIZE.x || coord.y >= VIEWPORT_SIZE.y)
			return;

		vec2 uv = vec2(coord) / vec2(VIEWPORT_SIZE);

		uint pixel = 0u;
		float depth = texture(depth_tex, uv).r;

		if(depth < 0.99)
		{
			const uint end = 1u << 24;
			vec3 pos = pixel_pos(vec3(uv, depth));

			for(int i = 0; i < lights.length(); i++)
			{
				vec4 l = lights[i];

				if(distance(l.xyz, pos) < l.w)
				{
					pixel = (pixel << 8) | uint(i + 1);

					if(pixel >= end)
						break;
				}
			}
		}

		imageStore(output_tex, coord, uvec4(pixel));
	}