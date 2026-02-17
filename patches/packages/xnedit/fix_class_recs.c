/*
 * fix_class_recs.c - Patch widget class record pointers broken by IRIX rld
 *
 * IRIX rld fails to apply some R_MIPS_REL32 relocations for external symbols
 * in static data of large cross-compiled executables. This leaves superclass
 * pointers and _XtInherit method pointers as NULL in widget ClassRec
 * structures, causing SIGBUS/errors when Xt tries to initialize the widget
 * class hierarchy.
 *
 * This constructor runs before main() and patches the affected class records
 * by reading the symbol values via GOT references (which rld resolves correctly).
 *
 * xnedit has MORE ClassRec structures than nedit:
 *   xmlFolderClassRec          (Microline/XmL/Folder.c) — Manager subclass
 *   xmlGridClassRec            (Microline/XmL/Grid.c) — Manager subclass
 *   xmlTreeClassRec            (Microline/XmL/Tree.c) — Grid subclass
 *   xmlProgressClassRec        (Microline/XmL/Progress.c) — Primitive subclass
 *   xrwsBubbleButtonClassRec   (Xlt/BubbleButton.c) — PushButton subclass
 *   tfWidgetClassRec           (util/textfield.c) — Primitive subclass
 *   xltSlideContextClassRec    (Xlt/SlideC.c) — Object subclass
 */

#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/PushBP.h>
#include <Xm/LabelP.h>

/* _XtInherit and _XtInheritTranslations from libXt.so */
extern void _XtInherit(void);
extern int _XtInheritTranslations;

/* Widget class records — declared as parent types to avoid private headers */
extern XmManagerClassRec xmlFolderClassRec;       /* XmLFolderClassRec */
extern XmManagerClassRec xmlGridClassRec;         /* XmLGridClassRec */
extern XmManagerClassRec xmlTreeClassRec;         /* XmLTreeClassRec */
extern XmPrimitiveClassRec xmlProgressClassRec;   /* XmLProgressClassRec */
extern XmPushButtonClassRec xrwsBubbleButtonClassRec;
extern XmPrimitiveClassRec tfWidgetClassRec;      /* TextFieldClassRec */
extern ObjectClassRec xltSlideContextClassRec;    /* XltSlideContextClassRec */

/* External class records from Motif — accessed via GOT (works) */
extern XmManagerClassRec xmManagerClassRec;
extern XmPrimitiveClassRec xmPrimitiveClassRec;
extern XmPushButtonClassRec xmPushButtonClassRec;
extern ObjectClassRec objectClassRec;

static void fix_class_superclass(void) __attribute__((constructor));

static void fix_class_superclass(void)
{
    /*
     * Fix xmlFolderClassRec (Folder.c) — Manager subclass
     * superclass = xmManagerClassRec
     */
    if (xmlFolderClassRec.core_class.superclass == (WidgetClass)0)
        xmlFolderClassRec.core_class.superclass = (WidgetClass)&xmManagerClassRec;
    if (xmlFolderClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xmlFolderClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (xmlFolderClassRec.composite_class.insert_child == (XtWidgetProc)0)
        xmlFolderClassRec.composite_class.insert_child = (XtWidgetProc)_XtInherit;
    if (xmlFolderClassRec.composite_class.delete_child == (XtWidgetProc)0)
        xmlFolderClassRec.composite_class.delete_child = (XtWidgetProc)_XtInherit;
    if (xmlFolderClassRec.manager_class.translations == (XtTranslations)0)
        xmlFolderClassRec.manager_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xmlFolderClassRec.manager_class.parent_process == (XmParentProcessProc)0)
        xmlFolderClassRec.manager_class.parent_process = (XmParentProcessProc)_XtInherit;

    /*
     * Fix xmlGridClassRec (Grid.c) — Manager subclass
     * superclass = xmManagerClassRec
     */
    if (xmlGridClassRec.core_class.superclass == (WidgetClass)0)
        xmlGridClassRec.core_class.superclass = (WidgetClass)&xmManagerClassRec;
    if (xmlGridClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xmlGridClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (xmlGridClassRec.composite_class.insert_child == (XtWidgetProc)0)
        xmlGridClassRec.composite_class.insert_child = (XtWidgetProc)_XtInherit;
    if (xmlGridClassRec.composite_class.delete_child == (XtWidgetProc)0)
        xmlGridClassRec.composite_class.delete_child = (XtWidgetProc)_XtInherit;
    if (xmlGridClassRec.manager_class.translations == (XtTranslations)0)
        xmlGridClassRec.manager_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xmlGridClassRec.manager_class.parent_process == (XmParentProcessProc)0)
        xmlGridClassRec.manager_class.parent_process = (XmParentProcessProc)_XtInherit;

    /*
     * Fix xmlTreeClassRec (Tree.c) — Grid subclass
     * superclass = xmlGridClassRec (internal, but may still be broken in larger binaries)
     */
    if (xmlTreeClassRec.core_class.superclass == (WidgetClass)0)
        xmlTreeClassRec.core_class.superclass = (WidgetClass)&xmlGridClassRec;
    if (xmlTreeClassRec.core_class.realize == (XtRealizeProc)0)
        xmlTreeClassRec.core_class.realize = (XtRealizeProc)_XtInherit;
    if (xmlTreeClassRec.core_class.resize == (XtWidgetProc)0)
        xmlTreeClassRec.core_class.resize = (XtWidgetProc)_XtInherit;
    if (xmlTreeClassRec.core_class.expose == (XtExposeProc)0)
        xmlTreeClassRec.core_class.expose = (XtExposeProc)_XtInherit;
    if (xmlTreeClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xmlTreeClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (xmlTreeClassRec.composite_class.geometry_manager == (XtGeometryHandler)0)
        xmlTreeClassRec.composite_class.geometry_manager = (XtGeometryHandler)_XtInherit;
    if (xmlTreeClassRec.composite_class.change_managed == (XtWidgetProc)0)
        xmlTreeClassRec.composite_class.change_managed = (XtWidgetProc)_XtInherit;
    if (xmlTreeClassRec.composite_class.insert_child == (XtWidgetProc)0)
        xmlTreeClassRec.composite_class.insert_child = (XtWidgetProc)_XtInherit;
    if (xmlTreeClassRec.composite_class.delete_child == (XtWidgetProc)0)
        xmlTreeClassRec.composite_class.delete_child = (XtWidgetProc)_XtInherit;
    if (xmlTreeClassRec.manager_class.translations == (XtTranslations)0)
        xmlTreeClassRec.manager_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xmlTreeClassRec.manager_class.parent_process == (XmParentProcessProc)0)
        xmlTreeClassRec.manager_class.parent_process = (XmParentProcessProc)_XtInherit;

    /*
     * Fix xmlProgressClassRec (Progress.c) — Primitive subclass
     * superclass = xmPrimitiveClassRec
     */
    if (xmlProgressClassRec.core_class.superclass == (WidgetClass)0)
        xmlProgressClassRec.core_class.superclass = (WidgetClass)&xmPrimitiveClassRec;
    if (xmlProgressClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xmlProgressClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (xmlProgressClassRec.primitive_class.border_highlight == (XtWidgetProc)0)
        xmlProgressClassRec.primitive_class.border_highlight = (XtWidgetProc)_XtInherit;
    if (xmlProgressClassRec.primitive_class.border_unhighlight == (XtWidgetProc)0)
        xmlProgressClassRec.primitive_class.border_unhighlight = (XtWidgetProc)_XtInherit;
    if (xmlProgressClassRec.primitive_class.translations == (XtTranslations)0)
        xmlProgressClassRec.primitive_class.translations = (XtTranslations)&_XtInheritTranslations;

    /*
     * Fix xrwsBubbleButtonClassRec (BubbleButton.c) — PushButton subclass
     * superclass = xmPushButtonClassRec
     */
    if (xrwsBubbleButtonClassRec.core_class.superclass == (WidgetClass)0)
        xrwsBubbleButtonClassRec.core_class.superclass = (WidgetClass)&xmPushButtonClassRec;
    if (xrwsBubbleButtonClassRec.core_class.realize == (XtRealizeProc)0)
        xrwsBubbleButtonClassRec.core_class.realize = (XtRealizeProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.core_class.resize == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.core_class.resize = (XtWidgetProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.core_class.expose == (XtExposeProc)0)
        xrwsBubbleButtonClassRec.core_class.expose = (XtExposeProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xrwsBubbleButtonClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.core_class.query_geometry == (XtGeometryHandler)0)
        xrwsBubbleButtonClassRec.core_class.query_geometry = (XtGeometryHandler)_XtInherit;
    if (xrwsBubbleButtonClassRec.primitive_class.border_highlight == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.primitive_class.border_highlight = (XtWidgetProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.primitive_class.border_unhighlight == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.primitive_class.border_unhighlight = (XtWidgetProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.primitive_class.translations == (XtTranslations)0)
        xrwsBubbleButtonClassRec.primitive_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xrwsBubbleButtonClassRec.primitive_class.arm_and_activate == (XtActionProc)0)
        xrwsBubbleButtonClassRec.primitive_class.arm_and_activate = (XtActionProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.label_class.menuProcs == (XmMenuProc)0)
        xrwsBubbleButtonClassRec.label_class.menuProcs = (XmMenuProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.label_class.translations == (XtTranslations)0)
        xrwsBubbleButtonClassRec.label_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xrwsBubbleButtonClassRec.label_class.setOverrideCallback == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.label_class.setOverrideCallback = (XtWidgetProc)_XtInherit;

    /*
     * Fix tfWidgetClassRec (textfield.c) — Primitive subclass
     * superclass = xmPrimitiveClassRec
     */
    if (tfWidgetClassRec.core_class.superclass == (WidgetClass)0)
        tfWidgetClassRec.core_class.superclass = (WidgetClass)&xmPrimitiveClassRec;
    if (tfWidgetClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        tfWidgetClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;
    if (tfWidgetClassRec.core_class.query_geometry == (XtGeometryHandler)0)
        tfWidgetClassRec.core_class.query_geometry = (XtGeometryHandler)_XtInherit;
    if (tfWidgetClassRec.primitive_class.border_highlight == (XtWidgetProc)0)
        tfWidgetClassRec.primitive_class.border_highlight = (XtWidgetProc)_XtInherit;
    if (tfWidgetClassRec.primitive_class.border_unhighlight == (XtWidgetProc)0)
        tfWidgetClassRec.primitive_class.border_unhighlight = (XtWidgetProc)_XtInherit;

    /*
     * Fix xltSlideContextClassRec (SlideC.c) — Object subclass
     * superclass = objectClassRec
     * Was not affected in nedit but xnedit binary is larger
     */
    if (xltSlideContextClassRec.object_class.superclass == (WidgetClass)0)
        xltSlideContextClassRec.object_class.superclass = (WidgetClass)&objectClassRec;
}
