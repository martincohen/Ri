typedef struct RiPrinter {
    int indentation;
    CoCharArray* out;
    CoCharArray buffer;
} RiPrinter;

// TODO: Rework by using 'cbprintfv' and callback that applies formatting directly.

static inline void
riprinter_add_indentation_(RiPrinter* printer, int indentation)
{
    for (int i = 0; i < indentation; ++i) {
        cochararray_push(printer->out, CoS("  "));
    }
}

static inline void
riprinter_remove_indentation_(RiPrinter* printer, int indentation)
{
    indentation *= 2;
    for (; indentation; --indentation) {
        if (printer->out->count && coarray_get_last(printer->out) == ' ') {
            coarray_pop(printer->out);
        }
    }
}

void
riprinter_print(RiPrinter* printer, const char* format, ...)
{
    coarray_clear(&printer->buffer);
    va_list args;
    va_start(args, format);
    cochararray_push_fv(&printer->buffer, format, args);
    va_end(args);

#if 1
    for (intptr_t i = 0; i < printer->buffer.count; ++i)
    {
        char c = printer->buffer.items[i];
        switch (c) {
            case '\n':
                coarray_push(printer->out, '\n');
                riprinter_add_indentation_(printer, printer->indentation);
                break;
            case '\t':
                printer->indentation++;
                riprinter_add_indentation_(printer, 1);
                break;
            case '\b':
                printer->indentation--;
                riprinter_remove_indentation_(printer, 1);
                break;
            default:
                coarray_push(printer->out, c);
                break;
        }
    }
#else
    int indentation = printer->indentation;
    for (intptr_t i = 0; i < printer->buffer.count; ++i)
    {
        char c = printer->buffer.items[i];
        switch (c) {
            case '\n':
                coarray_push(printer->out, '\n');
                riprinter_add_indentation_(printer, indentation);
                break;
            case '\t':
                printer->indentation++;
                riprinter_add_indentation_(printer, 1);
                break;
            default:
                coarray_push(printer->out, c);
                break;
        }
    }
#endif
}

