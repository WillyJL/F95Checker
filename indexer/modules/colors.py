import functools


@functools.cache
def hex_to_rgba_0_1(hex: str):
    r = int(hex[1:3], base=16) / 255
    g = int(hex[3:5], base=16) / 255
    b = int(hex[5:7], base=16) / 255
    if len(hex) > 7:
        a = int(hex[7:9], base=16) / 255
    else:
        a = 1.0
    return (r, g, b, a)


@functools.cache
def rgba_0_1_to_hex(rgba: tuple[int, int, int, int | None]):
    r = "%.2x" % int(rgba[0] * 255)
    g = "%.2x" % int(rgba[1] * 255)
    b = "%.2x" % int(rgba[2] * 255)
    if len(rgba) > 3:
        a = "%.2x" % int(rgba[3] * 255)
    else:
        a = "FF"
    return f"#{r}{g}{b}{a}"


@functools.cache
def foreground_color(bg: tuple[float, float, float, float | None]):
    gamma = 2.2
    # https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    luma = 0.2126 * pow(bg[0], gamma) + 0.7152 * pow(bg[1], gamma) + 0.0722 * pow(bg[2], gamma)
    return (0.04, 0.04, 0.04, 1.0) if luma > pow(0.5, gamma) else (1.0, 1.0, 1.0, 1.0)
