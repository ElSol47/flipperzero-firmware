#include "widget.h"
#include "widget_elements/widget_element_i.h"
#include <furi.h>
#include <m-array.h>

ARRAY_DEF(ElementArray, WidgetElement*, M_PTR_OPLIST); // NOLINT

struct Widget {
    View* view;
    void* context;
};

typedef struct {
    ElementArray_t element;
} GuiWidgetModel;

static void gui_widget_view_draw_callback(Canvas* canvas, void* _model) {
    GuiWidgetModel* model = _model;
    canvas_clear(canvas);

    // Draw all elements
    ElementArray_it_t it;
    ElementArray_it(it, model->element);
    while(!ElementArray_end_p(it)) {
        WidgetElement* element = *ElementArray_ref(it);
        if(element->draw != NULL) {
            element->draw(canvas, element);
        }
        ElementArray_next(it);
    }
}

static bool gui_widget_view_input_callback(InputEvent* event, void* context) {
    Widget* widget = context;
    bool consumed = false;

    // Call all Widget Elements input handlers
    with_view_model(
        widget->view,
        GuiWidgetModel * model,
        {
            ElementArray_it_t it;
            ElementArray_it(it, model->element);
            while(!ElementArray_end_p(it)) {
                WidgetElement* element = *ElementArray_ref(it);
                if(element->input != NULL) {
                    consumed |= element->input(event, element);
                }
                ElementArray_next(it);
            }
        },
        true);

    return consumed;
}

Widget* widget_alloc(void) {
    Widget* widget = malloc(sizeof(Widget));
    widget->view = view_alloc();
    view_set_context(widget->view, widget);
    view_allocate_model(widget->view, ViewModelTypeLocking, sizeof(GuiWidgetModel));
    view_set_draw_callback(widget->view, gui_widget_view_draw_callback);
    view_set_input_callback(widget->view, gui_widget_view_input_callback);

    with_view_model(
        widget->view, GuiWidgetModel * model, { ElementArray_init(model->element); }, true);

    return widget;
}

void widget_reset(Widget* widget) {
    furi_check(widget);

    with_view_model(
        widget->view,
        GuiWidgetModel * model,
        {
            ElementArray_it_t it;
            ElementArray_it(it, model->element);
            while(!ElementArray_end_p(it)) {
                WidgetElement* element = *ElementArray_ref(it);
                furi_assert(element->free);
                element->free(element);
                ElementArray_next(it);
            }
            ElementArray_reset(model->element);
        },
        true);
}

void widget_free(Widget* widget) {
    furi_check(widget);
    // Free all elements
    widget_reset(widget);
    // Free elements container
    with_view_model(
        widget->view, GuiWidgetModel * model, { ElementArray_clear(model->element); }, true);

    view_free(widget->view);
    free(widget);
}

View* widget_get_view(Widget* widget) {
    furi_check(widget);
    return widget->view;
}

static void widget_add_element(Widget* widget, WidgetElement* element) {
    furi_assert(widget);
    furi_assert(element);

    with_view_model(
        widget->view,
        GuiWidgetModel * model,
        {
            element->parent = widget;
            ElementArray_push_back(model->element, element);
        },
        true);
}

void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    furi_check(widget);
    WidgetElement* string_multiline_element =
        widget_element_string_multiline_create(x, y, horizontal, vertical, font, text);
    widget_add_element(widget, string_multiline_element);
}

void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    furi_check(widget);
    WidgetElement* string_element =
        widget_element_string_create(x, y, horizontal, vertical, font, text);
    widget_add_element(widget, string_element);
}

void widget_add_text_box_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots) {
    furi_check(widget);
    WidgetElement* text_box_element = widget_element_text_box_create(
        x, y, width, height, horizontal, vertical, text, strip_to_dots);
    widget_add_element(widget, text_box_element);
}

void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text) {
    furi_check(widget);
    WidgetElement* text_scroll_element =
        widget_element_text_scroll_create(x, y, width, height, text);
    widget_add_element(widget, text_scroll_element);
}

void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context) {
    furi_check(widget);
    WidgetElement* button_element =
        widget_element_button_create(button_type, text, callback, context);
    widget_add_element(widget, button_element);
}

void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon) {
    furi_check(widget);
    furi_check(icon);
    WidgetElement* icon_element = widget_element_icon_create(x, y, icon);
    widget_add_element(widget, icon_element);
}

void widget_add_rect_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius,
    bool fill) {
    furi_check(widget);
    WidgetElement* rect_element = widget_element_rect_create(x, y, width, height, radius, fill);
    widget_add_element(widget, rect_element);
}

void widget_add_circle_element(Widget* widget, uint8_t x, uint8_t y, uint8_t radius, bool fill) {
    furi_check(widget);
    WidgetElement* circle_element = widget_element_circle_create(x, y, radius, fill);
    widget_add_element(widget, circle_element);
}

void widget_add_line_element(Widget* widget, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    furi_check(widget);
    WidgetElement* line_element = widget_element_line_create(x1, y1, x2, y2);
    widget_add_element(widget, line_element);
}
