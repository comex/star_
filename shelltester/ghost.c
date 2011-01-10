#include <QuartzCore/QuartzCore.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: ghost <font> <output.png>\n");
        return 1;
    }
    CGFontRef font = CGFontCreateWithDataProvider(CGDataProviderCreateWithFilename(argv[1]));
    CFShow(font);
    CGContextRef context = CGBitmapContextCreate(NULL, 640, 480, 8, 640*4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast);
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    CGContextFillRect(context, CGRectMake(0, 0, 640, 480));
    CGContextSetRGBFillColor(context, 0, 0, 0, 1);
    CGContextSetRGBStrokeColor(context, 1, 0, 0, 1);
    CGContextSetFont(context, font);
    CGContextSetFontSize(context, 48);
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    CGGlyph glyph;
    for(glyph = 0; glyph < CGFontGetNumberOfGlyphs(font); glyph++) {
        printf("%d\n", glyph);
        CGContextSetTextPosition(context, 320, 240);
        CGContextShowGlyphs(context, &glyph, 1);
    }

    if(argc == 3) {
        CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingASCII), kCFURLPOSIXPathStyle, false);
        CGImageDestinationRef dest = CGImageDestinationCreateWithURL(url, CFSTR("public.png"), 1, NULL);
        CGImageDestinationAddImage(dest, CGBitmapContextCreateImage(context), NULL);
        CGImageDestinationFinalize(dest);
        CFRelease(dest);
    }
    return 0;
}
