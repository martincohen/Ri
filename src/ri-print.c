typedef struct RiPrinter {
    int indentation;
    CharArray* out;
    CharArray buffer;
} RiPrinter;

// TODO: Rework by using 'cbprintfv' and callback that applies formatting directly.

static inline void
riprinter_add_indentation_(RiPrinter* printer, int indentation)
{
    for (int i = 0; i < indentation; ++i) {
        chararray_push(printer->out, S("  "));
    }
}

static inline void
riprinter_remove_indentation_(RiPrinter* printer, int indentation)
{
    indentation *= 2;
    for (; indentation; --indentation) {
        if (printer->out->count && array_last(printer->out) == ' ') {
            array_pop(printer->out);
        }
    }
}

void
riprinter_print(RiPrinter* printer, const char* format, ...)
{
    array_clear(&printer->buffer);
    va_list args;
    va_start(args, format);
    chararray_push_fv(&printer->buffer, format, args);
    va_end(args);

#if 1
    for (iptr i = 0; i < printer->buffer.count; ++i)
    {
        char c = printer->buffer.items[i];
        switch (c) {
            case '\n':
                array_push(printer->out, '\n');
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
                array_push(printer->out, c);
                break;
        }
    }
#else
    int indentation = printer->indentation;
    for (iptr i = 0; i < printer->buffer.count; ++i)
    {
        char c = printer->buffer.items[i];
        switch (c) {
            case '\n':
                array_push(printer->out, '\n');
                riprinter_add_indentation_(printer, indentation);
                break;
            case '\t':
                printer->indentation++;
                riprinter_add_indentation_(printer, 1);
                break;
            default:
                array_push(printer->out, c);
                break;
        }
    }
#endif
}

