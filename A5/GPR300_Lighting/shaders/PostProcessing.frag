#version 450

out vec4 FragColor;
in vec2 texCoords;

//this *would* be a bool but we can't have nice things.
uniform int _UseEffect;
uniform int _SelectedEffect;
uniform sampler2D _RenderedTex;
uniform int _Width;
uniform int _Height;
uniform float _Time;


void main()
{

	if(_UseEffect == 0)
	{
		FragColor = texture(_RenderedTex, texCoords);
	}
	else
	{
		vec4 screenColor = texture(_RenderedTex, texCoords);
		float saturation = (screenColor.x + screenColor.y + screenColor.z);
		switch(_SelectedEffect)
		{
			case 1:
				FragColor = vec4(saturation, saturation, saturation, 1);
				break;
			case 2:
				vec2 uv = vec2(gl_FragCoord.x, gl_FragCoord.y) / vec2(_Width, _Height);
				uv.y += ((sin((uv.x + _Time) * 1.5f) + 1.0) / 2.0f );
				FragColor = texture(_RenderedTex, uv);
				break;
			case 3:
				float blendOne = smoothstep(0.3, 0.65, saturation);
				float blendTwo = smoothstep(0.65, 1.0, saturation);
				FragColor = mix(vec4(0, 0, 1, 1),  vec4(0, 1, 0, 1), blendOne);
				FragColor = mix(FragColor, vec4(1, 0, 0, 1), blendTwo);
				break;
		}

		
	}
}