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
 * Affected class records in nedit:
 *   xmlFolderClassRec          (Microline/XmL/Folder.c)
 *   xrwsBubbleButtonClassRec   (Xlt/BubbleButton.c)
 *
 * NOT affected (resolved correctly by rld):
 *   textClassRec               (source/text.c)
 *   xltSlideContextClassRec    (Xlt/SlideC.c)
 */

#include <X11/IntrinsicP.h>
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/PushBP.h>
#include <Xm/LabelP.h>

/* _XtInherit and _XtInheritTranslations from libXt.so */
extern void _XtInherit(void);
extern int _XtInheritTranslations;

/* Widget class records defined in nedit's source tree */
extern XmManagerClassRec xmlFolderClassRec;      /* actually XmLFolderClassRec, but we only access manager-level fields */
extern XmPushButtonClassRec xrwsBubbleButtonClassRec;

/* External class records from Motif - accessed via GOT (works) */
extern XmManagerClassRec xmManagerClassRec;
extern XmPushButtonClassRec xmPushButtonClassRec;

/* Macros to avoid repeating the _XtInherit cast */
#define INHERIT  ((XtProc)_XtInherit)

static void fix_class_superclass(void) __attribute__((constructor));

static void fix_class_superclass(void)
{
    /*
     * Fix xmlFolderClassRec — all fields that reference _XtInherit or
     * external class records. These match the initializer in Folder.c.
     */

    /* superclass → xmManagerClassRec */
    if (xmlFolderClassRec.core_class.superclass == (WidgetClass)0)
        xmlFolderClassRec.core_class.superclass = (WidgetClass)&xmManagerClassRec;

    /* core_class.set_values_almost → XtInheritSetValuesAlmost */
    if (xmlFolderClassRec.core_class.set_values_almost == (XtAlmostProc)0)
        xmlFolderClassRec.core_class.set_values_almost = (XtAlmostProc)_XtInherit;

    /* composite_class.insert_child → XtInheritInsertChild */
    if (xmlFolderClassRec.composite_class.insert_child == (XtWidgetProc)0)
        xmlFolderClassRec.composite_class.insert_child = (XtWidgetProc)_XtInherit;

    /* composite_class.delete_child → XtInheritDeleteChild */
    if (xmlFolderClassRec.composite_class.delete_child == (XtWidgetProc)0)
        xmlFolderClassRec.composite_class.delete_child = (XtWidgetProc)_XtInherit;

    /* manager_class.translations → XtInheritTranslations */
    if (xmlFolderClassRec.manager_class.translations == (XtTranslations)0)
        xmlFolderClassRec.manager_class.translations = (XtTranslations)&_XtInheritTranslations;

    /* manager_class.parent_process → XmInheritParentProcess */
    if (xmlFolderClassRec.manager_class.parent_process == (XmParentProcessProc)0)
        xmlFolderClassRec.manager_class.parent_process = (XmParentProcessProc)_XtInherit;

    /*
     * Fix xrwsBubbleButtonClassRec — all _XtInherit fields.
     * These match the initializer in Xlt/BubbleButton.c.
     */

    /* superclass → xmPushButtonClassRec */
    if (xrwsBubbleButtonClassRec.core_class.superclass == (WidgetClass)0)
        xrwsBubbleButtonClassRec.core_class.superclass = (WidgetClass)&xmPushButtonClassRec;

    /* core: realize, resize, expose, set_values_almost, query_geometry */
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

    /* primitive: border_highlight, border_unhighlight, arm_and_activate */
    if (xrwsBubbleButtonClassRec.primitive_class.border_highlight == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.primitive_class.border_highlight = (XtWidgetProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.primitive_class.border_unhighlight == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.primitive_class.border_unhighlight = (XtWidgetProc)_XtInherit;
    /* primitive translations */
    if (xrwsBubbleButtonClassRec.primitive_class.translations == (XtTranslations)0)
        xrwsBubbleButtonClassRec.primitive_class.translations = (XtTranslations)&_XtInheritTranslations;
    if (xrwsBubbleButtonClassRec.primitive_class.arm_and_activate == (XtActionProc)0)
        xrwsBubbleButtonClassRec.primitive_class.arm_and_activate = (XtActionProc)_XtInherit;

    /* label: menuProcs, translations */
    if (xrwsBubbleButtonClassRec.label_class.menuProcs == (XmMenuProc)0)
        xrwsBubbleButtonClassRec.label_class.menuProcs = (XmMenuProc)_XtInherit;
    if (xrwsBubbleButtonClassRec.label_class.translations == (XtTranslations)0)
        xrwsBubbleButtonClassRec.label_class.translations = (XtTranslations)&_XtInheritTranslations;
    /* setOverrideCallback is _XtInherit cast */
    if (xrwsBubbleButtonClassRec.label_class.setOverrideCallback == (XtWidgetProc)0)
        xrwsBubbleButtonClassRec.label_class.setOverrideCallback = (XtWidgetProc)_XtInherit;
}
