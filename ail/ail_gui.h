// Utility for simple, good Graphical User Interfaces in Raylib
//
// Define AIL_GUI_IMPL in one file
// Overwrite the static ail_gui_allocator variable, to use a different allocator
//
// LICENSE
/*
Copyright (c) 2024 Val Richter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef AIL_GUI_H_
#define AIL_GUI_H_

// @TODO: Move this to a separate repo as its own, standalone, single-header library, since I have been copy-pasting this same code between different projects already

#define AIL_ALL_IMPL
#include "ail.h"
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include "raylib.h"
#include "rlgl.h"

#ifndef AIL_GUI_DEF
#ifdef  AIL_DEF
#define AIL_GUI_DEF AIL_DEF
#else
#define AIL_GUI_DEF
#endif // AIL_DEF
#endif // AIL_GUI_DEF
#ifndef AIL_GUI_DEF_INLINE
#ifdef  AIL_DEF_INLINE
#define AIL_GUI_DEF_INLINE AIL_DEF_INLINE
#else
#define AIL_GUI_DEF_INLINE static inline
#endif // AIL_DEF_INLINE
#endif // AIL_GUI_DEF_INLINE

#ifndef AIL_GUI_SET_CURSOR
#define AIL_GUI_SET_CURSOR(c) SetMouseCursor(c)
#endif

typedef enum {
    AIL_GUI_STATE_HIDDEN,   // Element is not displayed
    AIL_GUI_STATE_INACTIVE, // Element is not active in any way. `state > AIL_GUI_STATE_INACTIVE` can be used to check if the element is active in anyway
    AIL_GUI_STATE_HOVERED,  // Element is currently being hovered
    AIL_GUI_STATE_PRESSED,  // Element is just being clicked on in this frame. It is active from now on
    AIL_GUI_STATE_FOCUSED,  // Element is being focused on and active. To be active is to accept user input
} AIL_Gui_State;

typedef enum {
    AIL_GUI_ALIGN_LT, // Align left and/or top
    AIL_GUI_ALIGN_C,  // Align center
    AIL_GUI_ALIGN_RB, // Align right and/or bottom
} AIL_Gui_Align;

typedef struct {
    RL_Color bg;           // RL_Color for background
    RL_Color border_color; // RL_Color for border
    i32   border_width; // Width of border
    // The following is only relevant for text
    RL_Color color;     // RL_Color for text
    RL_Font  font;      // The font used
    i32   pad;       // Padding; Space between text and border
    float font_size; // RL_Font Size
    float cSpacing;  // Spacing between characters of text
    float lSpacing;  // Spacing between lines of text
    AIL_Gui_Align hAlign; // Horizontal Text Alignment
    AIL_Gui_Align vAlign; // Vertical Text Alignment
} AIL_Gui_Style;

typedef struct {
    // @Note: All coordinates here are absolute and not relative to any bounding box
    const char *text;        // Text to be drawn
    AIL_DA(u16) lineOffsets; // Amount of chars until the next line should start ('\n' is ignored)
    AIL_DA(i32) lineXs;      // x coordinates of line starts (y coordinates come from AIL_Gui_Style)
    i32   y;                 // y coordinate of first line
    float w;                 // Width of text (Height can be calculated via amount of lines and style)
    u32   text_len;          // Amount of bytes in tet
} AIL_Gui_Drawable_Text;

typedef struct {
    u32 start;
    u32 end;
} AIL_Gui_Selection;

typedef struct {
    RL_Rectangle  bounds;
    AIL_DA(char)  text;
    AIL_Gui_Style defaultStyle;
    AIL_Gui_Style hovered;
} AIL_Gui_Label;

typedef struct {
    const char *placeholder; // Placeholder
    AIL_Gui_Label label;         // AIL_Gui_Label to display and update on input
    i32 cur;                 // Index in the text at which the cursor should be displayed
    i32 anim_idx;            // Current index in playing animation - if negative, it is currently in waiting time
    u16  rows;               // Amount of rows in label.text. If there's no newline, `rows == 1`
    bool resize;
    bool multiline;
    bool selected;
    // @TODO:
} AIL_Gui_Input_Box;

// @Memory: Store the boolean flags as a bitfield?
typedef struct {
    bool updated;
    bool enter;
    bool tab;
    bool escape;
    AIL_Gui_State state;
} AIL_Gui_Update_Res;

AIL_GUI_DEF_INLINE void ail_gui_setTextLineSpacing(i32 spacing);
AIL_GUI_DEF_INLINE bool ail_gui_stateIsActive(AIL_Gui_State state);
AIL_GUI_DEF_INLINE bool ail_gui_isPointInRec(i32 px, i32 py, i32 rx, i32 ry, i32 rw, i32 rh);
AIL_GUI_DEF AIL_Gui_Style ail_gui_defaultStyle(RL_Font font);
AIL_GUI_DEF AIL_Gui_Style ail_gui_cloneStyle(AIL_Gui_Style self);
AIL_GUI_DEF_INLINE void ail_gui_free_drawable_text(AIL_Gui_Drawable_Text *drawableText);
AIL_GUI_DEF AIL_Gui_Drawable_Text ail_gui_prepTextForDrawing(const char *text, RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF RL_Vector2 ail_gui_measureText(const char *text, RL_Rectangle bounds, AIL_Gui_Style style, AIL_Gui_Drawable_Text *drawable_text);
AIL_GUI_DEF void ail_gui_drawRectOuterBounds(RL_Rectangle rect, RL_Rectangle outer, RL_Color col);
AIL_GUI_DEF_INLINE i32 ail_gui_drawPreparedTextWithCursor(AIL_Gui_Drawable_Text text, AIL_Gui_Style style, bool outerBounds, RL_Rectangle outer, i32 cursor_idx, bool handle_mouse);
AIL_GUI_DEF void ail_gui_drawPreparedTextOuterBounds(AIL_Gui_Drawable_Text text, RL_Rectangle outer, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawPreparedText(AIL_Gui_Drawable_Text text, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawText(const char *text, RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawTextOuterBounds(const char *text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawBounds(RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawBoundsOuterBounds(RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style);
AIL_GUI_DEF RL_Rectangle ail_gui_getMinBounds(AIL_Gui_Drawable_Text text, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawPreparedSized(AIL_Gui_Drawable_Text text, RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawPreparedSizedOuterBounds(AIL_Gui_Drawable_Text text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawSized(const char *text, RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF void ail_gui_drawSizedOuterBounds(const char *text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style);
AIL_GUI_DEF RL_Vector2* ail_gui_drawSizedEx(AIL_Gui_Drawable_Text text, RL_Rectangle bounds, AIL_Gui_Style style);
AIL_GUI_DEF AIL_Gui_Label ail_gui_newLabel(RL_Rectangle bounds, char *text, AIL_Gui_Style defaultStyle, AIL_Gui_Style hovered);
AIL_GUI_DEF void ail_gui_freeLabel(AIL_Gui_Label *label);
AIL_GUI_DEF void ail_gui_rmCharLabel(AIL_Gui_Label *self, u32 idx);
AIL_GUI_DEF void ail_gui_insertCharLabel(AIL_Gui_Label *self, i32 idx, char c);
AIL_GUI_DEF void ail_gui_insertSliceLabel(AIL_Gui_Label *self, i32 idx, const char *slice, u32 slice_size);
AIL_GUI_DEF AIL_Gui_State ail_gui_getState(i32 x, i32 y, i32 w, i32 h);
AIL_GUI_DEF RL_Vector2 ail_gui_measureLabelText(AIL_Gui_Label self, AIL_Gui_State state);
AIL_GUI_DEF_INLINE void ail_gui_resizeLabel(AIL_Gui_Label *self, AIL_Gui_State state);
AIL_GUI_DEF AIL_Gui_Drawable_Text ail_gui_resizeLabelEx(AIL_Gui_Label *self, AIL_Gui_State state, const char *text);
AIL_GUI_DEF AIL_Gui_State ail_gui_drawLabel(AIL_Gui_Label self);
AIL_GUI_DEF AIL_Gui_State ail_gui_drawLabelOuterBounds(AIL_Gui_Label self, RL_Rectangle outer_bounds);
AIL_GUI_DEF AIL_Gui_Input_Box ail_gui_newInputBox(char *placeholder, bool resize, bool multiline, bool selected, AIL_Gui_Label label);
AIL_GUI_DEF bool ail_gui_isInputBoxHovered(AIL_Gui_Input_Box self);
AIL_GUI_DEF_INLINE AIL_Gui_State ail_gui_getInputBoxState(AIL_Gui_Input_Box *self);
AIL_GUI_DEF_INLINE AIL_Gui_State ail_gui_getInputBoxStateHelper(AIL_Gui_Input_Box *self, bool hovered);
AIL_GUI_DEF void ail_gui_resetInputBoxAnim(AIL_Gui_Input_Box *self);
AIL_GUI_DEF AIL_Gui_Update_Res ail_gui_handleKeysInputBox(AIL_Gui_Input_Box *self);
AIL_GUI_DEF AIL_Gui_Update_Res ail_gui_drawInputBox(AIL_Gui_Input_Box *self);

#endif // AIL_GUI_H_


#ifdef AIL_GUI_IMPL
#ifndef _GUI_IMPL_GUARD_
#define _GUI_IMPL_GUARD_

AIL_Allocator ail_gui_allocator = {
	.data       = NULL,
	.alloc      = &ail_default_malloc,
	.zero_alloc = &ail_default_calloc,
	.re_alloc   = &ail_default_realloc,
	.free_one   = &ail_default_free,
	.free_all   = &ail_default_free_all,
};

// Static Variables
static i16   Input_Box_anim_len  = 50;   // Length of animation in ms
static i16   Input_Box_anim_wait = 30;   // Time in ms to wait after the cursor moved before starting to animate again
static i32   Input_Box_cur_width = 4;    // Width of the displayed cursor
static RL_Color Input_Box_cur_color = { 0, 121, 241, 255 }; // RL_Color of the displayed cursor
static i32   text_line_spacing   = 15;   // Same as in raylib;

AIL_GUI_DEF_INLINE void ail_gui_setTextLineSpacing(i32 spacing)
{
    text_line_spacing = spacing;
    SetTextLineSpacing(spacing);
}

AIL_GUI_DEF_INLINE bool ail_gui_stateIsActive(AIL_Gui_State state)
{
    return state >= AIL_GUI_STATE_PRESSED;
}

AIL_GUI_DEF_INLINE bool ail_gui_isPointInRec(i32 px, i32 py, i32 rx, i32 ry, i32 rw, i32 rh)
{
    return (px >= rx) && (px <= rx + rw) && (py >= ry) && (py <= ry + rh);
}

AIL_GUI_DEF AIL_Gui_Style ail_gui_defaultStyle(RL_Font font)
{
    return (AIL_Gui_Style) {
        .bg           = RL_BLANK,
        .border_color = RL_BLANK,
        .border_width = 0,
        .color        = RL_WHITE,
        .pad          = 0,
        .font         = font,
        .font_size    = 30,
        .cSpacing     = 0,
        .lSpacing     = 5,
        .hAlign       = AIL_GUI_ALIGN_LT,
        .vAlign       = AIL_GUI_ALIGN_LT,
    };
}

AIL_GUI_DEF AIL_Gui_Style ail_gui_cloneStyle(AIL_Gui_Style self)
{
    return (AIL_Gui_Style) {
        .bg           = self.bg,
        .border_color = self.border_color,
        .border_width = self.border_width,
        .color        = self.color,
        .pad          = self.pad,
        .font         = self.font,
        .font_size    = self.font_size,
        .cSpacing     = self.cSpacing,
        .lSpacing     = self.lSpacing,
        .hAlign       = self.hAlign,
        .vAlign       = self.vAlign,
    };
}

AIL_GUI_DEF_INLINE void ail_gui_free_drawable_text(AIL_Gui_Drawable_Text *drawableText)
{
    ail_da_free(&drawableText->lineOffsets);
    ail_da_free(&drawableText->lineXs);
}

// @Important This function allocates memory. The returned AIL_Gui_Drawable_Text instance has to be freed again via ail_gui_free_drawable_text
AIL_GUI_DEF AIL_Gui_Drawable_Text ail_gui_prepTextForDrawing(const char *text, RL_Rectangle bounds, AIL_Gui_Style style)
{
    if (!text) return (AIL_Gui_Drawable_Text) {0};
    float y = bounds.y + style.pad;
    AIL_Gui_Drawable_Text out = {0};
    out.text = text;
    out.y    = y;
    out.lineOffsets = ail_da_new_with_alloc(u16, 16, &ail_gui_allocator);
    out.lineXs      = ail_da_new_with_alloc(i32, 17, &ail_gui_allocator);

    float scaleFactor = style.font_size / (float)style.font.baseSize; // Character quad scaling factor
    float lineWidth   = 0.0f;
    u16   lineOffset  = 0;

    i32 cp;        // Current codepoint
    i32 cpSize;    // Current codepoint size in bytes
    float cpWidth; // Width of current codepoint
    for (; (cp = GetCodepointNext(text, &cpSize)); text += cpSize, out.text_len += cpSize, lineOffset += cpSize) {
        if (cp == '\n') {
            ail_da_push(&out.lineOffsets, lineOffset);
            if (lineWidth > out.w) out.w = lineWidth;
            switch (style.hAlign) {
                case AIL_GUI_ALIGN_LT:
                    ail_da_push(&out.lineXs, bounds.x + style.pad);
                    break;
                case AIL_GUI_ALIGN_C:
                    ail_da_push(&out.lineXs, bounds.x + (bounds.width - lineWidth)/2.0f);
                    break;
                case AIL_GUI_ALIGN_RB:
                    ail_da_push(&out.lineXs, bounds.x + bounds.width - lineWidth);
                    break;
            }
            y += style.font_size + style.lSpacing;
            lineOffset = 0;
            lineWidth  = 0;
        }
        else {
            i32 glyphIndex = GetGlyphIndex(style.font, cp);
            float w = style.font.glyphs[glyphIndex].advanceX ? style.font.glyphs[glyphIndex].advanceX : style.font.recs[glyphIndex].width;
            cpWidth = style.cSpacing + scaleFactor*w;

            // @TODO: When splittin text into new lines, it would be nice to split text by words instead of by characters
            if (lineWidth + cpWidth > bounds.width - style.pad) {
                ail_da_push(&out.lineOffsets, lineOffset - 1);
                if (lineWidth > out.w) out.w = lineWidth;
                switch (style.hAlign) {
                    case AIL_GUI_ALIGN_LT:
                        ail_da_push(&out.lineXs, bounds.x + style.pad);
                        break;
                    case AIL_GUI_ALIGN_C:
                        ail_da_push(&out.lineXs, bounds.x + (bounds.width - lineWidth)/2.0f);
                        break;
                    case AIL_GUI_ALIGN_RB:
                        ail_da_push(&out.lineXs, bounds.x + bounds.width - lineWidth - style.pad);
                        break;
                }
                y += style.font_size + style.lSpacing;
                lineOffset = 1;
                lineWidth  = cpWidth;
            }
            else {
                lineWidth += cpWidth;
            }
        }
    }

    // @Cleanup: Almost identical code with switch-case here 3 times
    if (lineWidth > out.w) out.w = lineWidth;
    switch (style.hAlign) {
        case AIL_GUI_ALIGN_LT:
            ail_da_push(&out.lineXs, bounds.x + style.pad);
            break;
        case AIL_GUI_ALIGN_C:
            ail_da_push(&out.lineXs, bounds.x + (bounds.width - lineWidth)/2.0f);
            break;
        case AIL_GUI_ALIGN_RB:
            ail_da_push(&out.lineXs, bounds.x + bounds.width - lineWidth);
            break;
    }

    // @TODO: Check for case where text is too big for bounding box
    float height = y - out.y;
    if (lineOffset) height += style.font_size;
    switch (style.vAlign) {
        case AIL_GUI_ALIGN_LT:
            break;
        case AIL_GUI_ALIGN_C:
            out.y = bounds.y + (bounds.height - height)/2.0f;
            break;
        case AIL_GUI_ALIGN_RB:
            out.y = bounds.y + bounds.height - height - style.pad;
            break;
    }

    return out;
}

// drawable_text will be written to, except if text == drawable_text.text
// @Important: This function potentially allocates memory for the drawable_text, which should be freed again via ail_gui_free_drawable_text
AIL_GUI_DEF RL_Vector2 ail_gui_measureText(const char *text, RL_Rectangle bounds, AIL_Gui_Style style, AIL_Gui_Drawable_Text *drawable_text)
{
    if (text != drawable_text->text || drawable_text->lineXs.data == NULL) *drawable_text = ail_gui_prepTextForDrawing(text, bounds, style);
    float height = drawable_text->lineXs.len*style.font_size + (drawable_text->lineXs.len - 1)*style.lSpacing;
    RL_Vector2 out = {
        .x = drawable_text->w,
        .y = height,
    };
    return out;
}

AIL_GUI_DEF void ail_gui_drawText(const char *text, RL_Rectangle bounds, AIL_Gui_Style style)
{
    AIL_Gui_Drawable_Text preppedText = ail_gui_prepTextForDrawing(text, bounds, style);
    ail_gui_drawPreparedText(preppedText, style);
    ail_gui_free_drawable_text(&preppedText);
}

AIL_GUI_DEF void ail_gui_drawTextOuterBounds(const char *text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style)
{
    AIL_Gui_Drawable_Text preppedText = ail_gui_prepTextForDrawing(text, inner, style);
    ail_gui_drawPreparedTextOuterBounds(preppedText, outer, style);
    ail_gui_free_drawable_text(&preppedText);
}

AIL_GUI_DEF void ail_gui_drawTextCodepointOuterBounds(RL_Font font, i32 codepoint, RL_Vector2 position, RL_Rectangle outer, f32 fontSize, RL_Color col)
{
    // @Note: Code adapted from raylib/rtext.c/DrawTextCodepoint
    i32 index = GetGlyphIndex(font, codepoint);
    f32 scaleFactor = fontSize/font.baseSize;

    // All rectangles here are given via the top-left and bottom-right coordinates, which will be <rect>1<x/y> and <rect>2<x/y> respectively
    // 'd' is the destination, 's' the source, 'p' the actually displayed destination and 'q' the actually displayed source
    // Further, 't' are the interpolation variables
    f32 d1x = position.x + font.glyphs[index].offsetX*scaleFactor - (f32)font.glyphPadding*scaleFactor;
    f32 d1y = position.y + font.glyphs[index].offsetY*scaleFactor - (f32)font.glyphPadding*scaleFactor;
    f32 d2x = d1x + (font.recs[index].width  + 2.0f*font.glyphPadding)*scaleFactor;
    f32 d2y = d1y + (font.recs[index].height + 2.0f*font.glyphPadding)*scaleFactor;

    f32 s1x = font.recs[index].x - (f32)font.glyphPadding;
    f32 s1y = font.recs[index].y - (f32)font.glyphPadding;
    f32 s2x = s1x + font.recs[index].width  + 2.0f*font.glyphPadding;
    f32 s2y = s1y + font.recs[index].height + 2.0f*font.glyphPadding;

    i32 p1x = AIL_MAX(outer.x, d1x);
    i32 p1y = AIL_MAX(outer.y, d1y);
    i32 p2x = AIL_MIN(outer.x + outer.width,  d2x);
    i32 p2y = AIL_MIN(outer.y + outer.height, d2y);

    f32 t1x = AIL_REV_LERP(p1x, d1x, d2x);
    f32 t1y = AIL_REV_LERP(p1y, d1y, d2y);
    f32 t2x = AIL_REV_LERP(p2x, d1x, d2x);
    f32 t2y = AIL_REV_LERP(p2y, d1y, d2y);

    i32 q1x = AIL_LERP(t1x, s1x, s2x);
    i32 q1y = AIL_LERP(t1y, s1y, s2y);
    i32 q2x = AIL_LERP(t2x, s1x, s2x);
    i32 q2y = AIL_LERP(t2y, s1y, s2y);

    RL_Rectangle src = { q1x, q1y, q2x - q1x, q2y - q1y };
    RL_Rectangle dst = { p1x, p1y, p2x - p1x, p2y - p1y };

    DrawTexturePro(font.texture, src, dst, (RL_Vector2){0.0f, 0.0f}, 0.0f, col);
}

AIL_GUI_DEF void ail_gui_drawRectOuterBounds(RL_Rectangle rect, RL_Rectangle outer, RL_Color col)
{
    i32 p1x = AIL_MAX(outer.x, rect.x);
    i32 p1y = AIL_MAX(outer.y, rect.y);
    i32 p2x = AIL_MIN(outer.x + outer.width,  rect.x + rect.width);
    i32 p2y = AIL_MIN(outer.y + outer.height, rect.y + rect.height);
    DrawRectangle(p1x, p1y, p2x - p1x, p2y - p1y, col);
}

AIL_GUI_DEF_INLINE i32 ail_gui_drawPreparedTextWithCursor(AIL_Gui_Drawable_Text text, AIL_Gui_Style style, bool outerBounds, RL_Rectangle outer, i32 cursor_idx, bool handle_mouse)
{
    if (!text.text) return -1;
    RL_Vector2 mouse  = GetMousePosition();
    float scaleFactor = style.font_size/style.font.baseSize; // Character quad scaling factor
    RL_Vector2 pos    = { .x = text.lineXs.data[0], .y = text.y }; // Position to draw current codepoint at
    u32 lastOffset    = 0;
    i32 cp;        // Current codepoint
    i32 cpSize;    // Current codepoint size in bytes
    i32 rune_idx   = 0;
    i32 new_idx    = -1;
    for (u32 i = 0, lineIdx = 0, xIdx = 1; (cp = GetCodepointNext(&text.text[i], &cpSize)) != 0; i += cpSize, rune_idx++) {
        if ((cp != '\n') && (cp != ' ') && (cp != '\t') && (cp != '\r')) {
            if (outerBounds) ail_gui_drawTextCodepointOuterBounds(style.font, cp, pos, outer, style.font_size, style.color);
            else RL_DrawTextCodepoint(style.font, cp, pos, style.font_size, style.color);
        }
        if (rune_idx == cursor_idx) {
            if (outerBounds) ail_gui_drawRectOuterBounds((RL_Rectangle){ pos.x, pos.y, style.font_size, Input_Box_cur_width }, outer, Input_Box_cur_color);
            else DrawRectangle((i32) pos.x, (i32) pos.y, Input_Box_cur_width, (i32) style.font_size, Input_Box_cur_color);
        }

        i32 idx = GetGlyphIndex(style.font, cp);
        float w = style.font.glyphs[idx].advanceX ? style.font.glyphs[idx].advanceX : style.font.recs[idx].width;
        f32 width = style.cSpacing + scaleFactor*w;
        if (handle_mouse && (mouse.y <= pos.y + style.font_size) && (mouse.x <= pos.x + width)) {
            new_idx      = rune_idx;
            handle_mouse = false;

        }
        if (AIL_UNLIKELY(lineIdx < text.lineOffsets.len && i == lastOffset + (i32)text.lineOffsets.data[lineIdx])) {
            pos.y += style.font_size + style.lSpacing;
            pos.x  = text.lineXs.data[xIdx];
            xIdx++;
            lineIdx++;
            lastOffset = i;
        } else {
            pos.x += width;
        }
    }
    if (handle_mouse && (mouse.y <= pos.y + style.font_size) && (mouse.x >= pos.x)) new_idx = rune_idx;
    if (rune_idx == cursor_idx) DrawRectangle((i32) pos.x, (i32) pos.y, Input_Box_cur_width, (i32) style.font_size, Input_Box_cur_color);
    return (new_idx < 0) ? cursor_idx : new_idx;
}

AIL_GUI_DEF void ail_gui_drawPreparedText(AIL_Gui_Drawable_Text text, AIL_Gui_Style style)
{
    ail_gui_drawPreparedTextWithCursor(text, style, false, (RL_Rectangle){0}, -1, false);
}

AIL_GUI_DEF void ail_gui_drawPreparedTextOuterBounds(AIL_Gui_Drawable_Text text, RL_Rectangle outer, AIL_Gui_Style style)
{
    ail_gui_drawPreparedTextWithCursor(text, style, true, outer, -1, false);
}

AIL_GUI_DEF void ail_gui_drawBounds(RL_Rectangle bounds, AIL_Gui_Style style)
{
    if (style.border_width > 0) {
        RL_Rectangle border = {
            .x = bounds.x - style.border_width,
            .y = bounds.y - style.border_width,
            .width  = bounds.width + 2*style.border_width,
            .height = bounds.height + 2*style.border_width,
        };
        DrawRectangleLinesEx(border, style.border_width, style.border_color);
    }
    DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, style.bg);
}

AIL_GUI_DEF void ail_gui_drawBoundsOuterBounds(RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style)
{
    f32 p1x = AIL_MAX(inner.x, outer.x);
    f32 p1y = AIL_MAX(inner.y, outer.y);
    f32 p2x = AIL_MIN(inner.x + inner.width, outer.x + outer.width);
    f32 p2y = AIL_MIN(inner.y + inner.height, outer.y + outer.height);
    if (style.border_width > 0) {
        f32 q1x = AIL_MAX(inner.x - style.border_width, outer.x);
        f32 q1y = AIL_MAX(inner.y - style.border_width, outer.y);
        f32 q2x = AIL_MIN(inner.x + style.border_width + inner.width, outer.x + outer.width);
        f32 q2y = AIL_MIN(inner.y + style.border_width + inner.height, outer.y + outer.height);
        DrawRectangleLinesEx((RL_Rectangle){ q1x, q1y, q2x - q1x, q2y - q1y }, style.border_width, style.border_color);
    }
    DrawRectangle(p1x, p1y, p2x - p1x, p2y - p1y, style.bg);
}

AIL_GUI_DEF RL_Rectangle ail_gui_getMinBounds(AIL_Gui_Drawable_Text text, AIL_Gui_Style style)
{
    i32 x = text.lineXs.data[0];
    for (u32 i = 1; i < text.lineXs.len; i++) x = AIL_MIN(x, text.lineXs.data[i]);
    x -= style.pad;
    i32 y = text.y - style.pad;
    i32 w = text.w + 2*style.pad;
    i32 h = text.lineOffsets.len * style.lSpacing + text.lineXs.len * style.font_size + 2*style.pad;
    return (RL_Rectangle) {x, y, w, h};
}

AIL_GUI_DEF void ail_gui_drawPreparedSized(AIL_Gui_Drawable_Text text, RL_Rectangle bounds, AIL_Gui_Style style)
{
    ail_gui_drawBounds(bounds, style);
    ail_gui_drawPreparedText(text, style);
}

AIL_GUI_DEF void ail_gui_drawPreparedSizedOuterBounds(AIL_Gui_Drawable_Text text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style)
{
    ail_gui_drawBoundsOuterBounds(inner, outer, style);
    ail_gui_drawPreparedTextOuterBounds(text, outer, style);
}

AIL_GUI_DEF void ail_gui_drawSized(const char *text, RL_Rectangle bounds, AIL_Gui_Style style)
{
    ail_gui_drawBounds(bounds, style);
    ail_gui_drawText(text, bounds, style);
}

AIL_GUI_DEF void ail_gui_drawSizedOuterBounds(const char *text, RL_Rectangle inner, RL_Rectangle outer, AIL_Gui_Style style)
{
    ail_gui_drawBoundsOuterBounds(inner, outer, style);
    ail_gui_drawTextOuterBounds(text, inner, outer, style);
}

AIL_GUI_DEF AIL_Gui_Label ail_gui_newLabel(RL_Rectangle bounds, char *text, AIL_Gui_Style defaultStyle, AIL_Gui_Style hovered)
{
    i32 text_len = text == NULL ? 0 : TextLength(text);
    AIL_DA(char) arrList = ail_da_new_with_alloc(char, text_len + 1, &ail_gui_allocator);
    arrList.len = text_len + 1;
    if (text_len > 0) memcpy(arrList.data, text, text_len);
    arrList.data[text_len] = 0;

    return (AIL_Gui_Label) {
        // .x            = x - defaultStyle.pad,
        // .y            = y - defaultStyle.pad,
        // .w            = ((i32) size.x) + 2*defaultStyle.pad,
        // .h            = defaultStyle.font_size + 2*defaultStyle.pad,
        .bounds       = bounds,
        .text         = arrList,
        .defaultStyle = defaultStyle,
        .hovered      = hovered,
    };
}

AIL_GUI_DEF void ail_gui_freeLabel(AIL_Gui_Label *label)
{
    ail_da_free(&label->text);
}

AIL_GUI_DEF void ail_gui_rmCharLabel(AIL_Gui_Label *self, u32 idx)
{
    if (idx >= self->text.len) return;
    ail_da_rm(&self->text, idx);
    self->text.data[self->text.len] = '\0';
}

AIL_GUI_DEF void ail_gui_insertCharLabel(AIL_Gui_Label *self, i32 idx, char c)
{
    ail_da_insert(&self->text, idx, c);
}

AIL_GUI_DEF void ail_gui_insertSliceLabel(AIL_Gui_Label *self, i32 idx, const char *slice, u32 slice_size)
{
    ail_da_insertn(&self->text, idx, slice, slice_size);
}

AIL_GUI_DEF AIL_Gui_State ail_gui_getState(i32 x, i32 y, i32 w, i32 h)
{
    RL_Vector2 mouse = GetMousePosition();
    bool hovered   = ail_gui_isPointInRec((i32) mouse.x, (i32) mouse.y, x, y, w, h);
    if (hovered > 0) return (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ? AIL_GUI_STATE_PRESSED : AIL_GUI_STATE_HOVERED;
    else return AIL_GUI_STATE_INACTIVE;
}

AIL_GUI_DEF_INLINE void ail_gui_resizeLabel(AIL_Gui_Label *self, AIL_Gui_State state)
{
    ail_gui_resizeLabelEx(self, state, self->text.data);
}

AIL_GUI_DEF AIL_Gui_Drawable_Text ail_gui_resizeLabelEx(AIL_Gui_Label *self, AIL_Gui_State state, const char *text)
{
    AIL_Gui_Style style = (state >= AIL_GUI_STATE_HOVERED) ? self->hovered : self->defaultStyle;
    AIL_Gui_Drawable_Text drawable = {0};
    RL_Vector2 size  = ail_gui_measureText(text, self->bounds, style, &drawable);
    self->bounds.width  = size.x + 2*style.pad;
    self->bounds.height = size.y + 2*style.pad;
    return drawable;
}

AIL_GUI_DEF RL_Vector2 ail_gui_measureLabelText(AIL_Gui_Label self, AIL_Gui_State state)
{
    AIL_Gui_Style style = (state >= AIL_GUI_STATE_HOVERED) ? self.hovered : self.defaultStyle;
    return MeasureTextEx(style.font, self.text.data, style.font_size, style.cSpacing);
}

AIL_GUI_DEF AIL_Gui_State ail_gui_drawLabel(AIL_Gui_Label self)
{
    AIL_Gui_State state = ail_gui_getState(self.bounds.x, self.bounds.y, self.bounds.width, self.bounds.height);
    if (state == AIL_GUI_STATE_HIDDEN) return state;
    bool hovered = state >= AIL_GUI_STATE_HOVERED;
    AIL_Gui_Style style   = (hovered) ? self.hovered : self.defaultStyle;

    AIL_Gui_Drawable_Text prepText = ail_gui_prepTextForDrawing(self.text.data, self.bounds, style);
    ail_gui_drawPreparedSized(prepText, self.bounds, style);
    ail_gui_free_drawable_text(&prepText);
    if (hovered) AIL_GUI_SET_CURSOR(MOUSE_CURSOR_POINTING_HAND);
    return state;
}

AIL_GUI_DEF AIL_Gui_State ail_gui_drawLabelOuterBounds(AIL_Gui_Label self, RL_Rectangle outer_bounds)
{
    AIL_Gui_State state = ail_gui_getState(self.bounds.x, self.bounds.y, self.bounds.width, self.bounds.height);
    if (state == AIL_GUI_STATE_HIDDEN) return state;
    bool hovered = state >= AIL_GUI_STATE_HOVERED;
    AIL_Gui_Style style   = (hovered) ? self.hovered : self.defaultStyle;

    AIL_Gui_Drawable_Text prepText = ail_gui_prepTextForDrawing(self.text.data, self.bounds, style);
    ail_gui_drawPreparedSizedOuterBounds(prepText, self.bounds, outer_bounds, style);
    ail_gui_free_drawable_text(&prepText);
    if (hovered) AIL_GUI_SET_CURSOR(MOUSE_CURSOR_POINTING_HAND);
    return state;
}

AIL_GUI_DEF AIL_Gui_Input_Box ail_gui_newInputBox(char *placeholder, bool resize, bool multiline, bool selected, AIL_Gui_Label label)
{
    u16 rows = 1;
    u32 i = 0;
    while (i < label.text.len) {
        if (label.text.data[i] == '\n') rows += 1;
        i += 1;
    }

    return (AIL_Gui_Input_Box) {
        .placeholder = placeholder,
        .label       = label,
        .cur         = (label.text.len == 0) ? 0 : label.text.len - 1,
        .anim_idx    = 0,
        .rows        = rows,
        .resize      = resize,
        .multiline   = multiline,
        .selected    = selected,
    };
}

AIL_GUI_DEF bool ail_gui_isInputBoxHovered(AIL_Gui_Input_Box self)
{
    RL_Vector2 mouse = GetMousePosition();
    return ail_gui_isPointInRec((i32)mouse.x, (i32)mouse.y, self.label.bounds.x, self.label.bounds.y, self.label.bounds.width, self.label.bounds.height);
}

AIL_GUI_DEF_INLINE AIL_Gui_State ail_gui_getInputBoxState(AIL_Gui_Input_Box *self)
{
    return ail_gui_getInputBoxStateHelper(self, ail_gui_isInputBoxHovered(*self));
}

AIL_GUI_DEF AIL_Gui_State ail_gui_getInputBoxStateHelper(AIL_Gui_Input_Box *self, bool hovered)
{
    bool clicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    if (!hovered && clicked) {
        self->selected = false;
        return AIL_GUI_STATE_INACTIVE;
    } else if (clicked) {
        self->selected = true;
        return AIL_GUI_STATE_PRESSED;
    } else if (self->selected) {
        return AIL_GUI_STATE_FOCUSED;
    } else if (hovered) {
        return AIL_GUI_STATE_HOVERED;
    } else {
        return AIL_GUI_STATE_INACTIVE;
    }
}

AIL_GUI_DEF void ail_gui_resetInputBoxAnim(AIL_Gui_Input_Box *self)
{
    self->anim_idx = -Input_Box_anim_len;
}

// Draws the AIL_Gui_Input_Box, but also does more, like handling any user input if the box is active
AIL_GUI_DEF AIL_Gui_Update_Res ail_gui_drawInputBox(AIL_Gui_Input_Box *self)
{
    AIL_Gui_Update_Res res = {0};
    bool hovered  = ail_gui_isInputBoxHovered(*self);
    AIL_Gui_State state = ail_gui_getInputBoxStateHelper(self, hovered);
    if (state == AIL_GUI_STATE_HIDDEN) return res;
    AIL_Gui_Style style = (hovered || ail_gui_stateIsActive(state)) ? self->label.hovered : self->label.defaultStyle;

    const char *text = self->label.text.data;
    if (!text || !text[0]) {
        text = self->placeholder;
        if (style.color.a >= 80) style.color.a -= 80;
        else style.color.a /= 2;
    }

    AIL_Gui_Drawable_Text prepText = {0};
    if (self->selected) {
        if (IsKeyPressed(KEY_TAB)) {
            res.tab = true;
        }
        else if (IsKeyPressed(KEY_ENTER)) {
            res.enter = true;
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            self->cur = AIL_MAX(self->cur - 1, 0);
        }
        else if (IsKeyPressed(KEY_RIGHT)) {
            self->cur = AIL_MIN(self->cur + 1, (i32)self->label.text.len - 1);
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            self->cur = self->label.text.len - 1;
        }
        else if (IsKeyPressed(KEY_UP)) {
            self->cur = 0;
        }
        else if (IsKeyPressed(KEY_DELETE)) {
            ail_gui_rmCharLabel(&self->label, self->cur);
            res.updated = true;
        }
        else if (IsKeyPressed(KEY_BACKSPACE) && self->cur > 0) {
            self->cur -= 1;
            ail_gui_rmCharLabel(&self->label, self->cur);
            res.updated = true;
        } else {
            int cp = GetCharPressed();
            while (cp > 0) {
                ail_gui_insertCharLabel(&self->label, self->cur, (char)cp);
                self->cur += 1;
                cp >>= 8;
            }
            res.updated = true;
        }
    }
    if (res.updated && self->resize) prepText = ail_gui_resizeLabelEx(&self->label, state, text);

    if (!prepText.lineXs.len) prepText = ail_gui_prepTextForDrawing(text, self->label.bounds, style);
    ail_gui_drawBounds(self->label.bounds, style);
    i32 newCur = ail_gui_drawPreparedTextWithCursor(prepText, style, false, self->label.bounds, ail_gui_stateIsActive(state) ? self->cur : -1, IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
    if (newCur >= 0 && newCur != self->cur) {
        self->anim_idx = -Input_Box_anim_wait;
        self->cur = newCur;
    }

    ail_gui_free_drawable_text(&prepText);
    if (hovered) AIL_GUI_SET_CURSOR(MOUSE_CURSOR_IBEAM);
    res.state = state;
    return res;
}

#endif // _GUI_IMPL_GUARD_
#endif // AIL_GUI_IMPL