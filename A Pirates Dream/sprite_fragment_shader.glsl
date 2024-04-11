// Source code of fragment shader
#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec2 uv_interp;
in int greyscale;

// Texture sampler
uniform sampler2D onetex;

void main()
{
    // Sample texture
    vec4 color = texture2D(onetex, uv_interp);

    // Assign color to fragment
    /*
    if (greyscale == 1)
    {
        float gs = (color.r + color.g + color.b) / 3;
        gl_FragColor = vec4(gs, gs, gs, color.a);
    }
    else
    {
        gl_FragColor = vec4(color.r, color.g, color.b, color.a);
    }    
    */
    gl_FragColor = vec4(color.r, color.g, color.b, color.a);

    // Check for transparency
    if(color.a < 1.0)
    {
         discard;
    }
}
