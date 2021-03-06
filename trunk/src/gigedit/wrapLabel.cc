/* *************************************************************************
 * Copyright (c) 2005 VMware, Inc.
 * Copyright (c) 2011 - 2017 Andreas Persson
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * *************************************************************************/

#include "wrapLabel.hh"
#ifdef PANGOMM_HEADER_FILE
# include PANGOMM_HEADER_FILE(pangomm.h)
# include PANGOMM_HEADER_FILE(pangommconfig.h)
#else
# include <pangomm.h>
# include <pangommconfig.h>
#endif

// we don't need this WrapLabel helper class on GTKMM 3 and higher
#if GTKMM_MAJOR_VERSION < 3

/*
 * wrapLabel.cc --
 *
 *     A wrappable label widget.
 */


namespace view {


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::WrapLabel --
 *
 *      Constructor.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

WrapLabel::WrapLabel(const Glib::ustring &text) // IN: The label text
   : mWrapWidth(0),
     mWrapHeight(0)
{
   // pangomm >= 2.35.1
#if PANGOMM_MAJOR_VERSION > 2 || (PANGOMM_MAJOR_VERSION == 2 && (PANGOMM_MINOR_VERSION > 35 || (PANGOMM_MINOR_VERSION == 35 && PANGOMM_MICRO_VERSION >= 1)))
   get_layout()->set_wrap(Pango::WrapMode::WORD_CHAR);
#else
   get_layout()->set_wrap(Pango::WRAP_WORD_CHAR);
#endif
   set_alignment(0.0, 0.0);
   set_text(text);
}


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::set_text --
 *
 *      Override function for Label::set_text() that re-sets the wrapping
 *      width after the text is set.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void
WrapLabel::set_text(const Glib::ustring &str) // IN: The text to set
{
   Label::set_text(str);

   SetWrapWidth(mWrapWidth);
}


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::set_markup --
 *
 *      Override function for Label::set_markup() that re-sets the wrapping
 *      width after the text is set.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void
WrapLabel::set_markup(const Glib::ustring &str) // IN: The text to set
{
   Label::set_markup(str);

   SetWrapWidth(mWrapWidth);
}


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::on_size_request --
 *
 *      Override handler for the "size_request" signal. Forces the height
 *      to be the size necessary for the Pango layout, while allowing the
 *      width to be flexible.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void
WrapLabel::on_size_request(Gtk::Requisition *req) // OUT: Our requested size
{
   req->width  = 0;
   req->height = mWrapHeight;
}


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::on_size_allocate --
 *
 *      Override handler for the "size_allocate" signal. Sets the wrap width
 *      to the be width allocated to us.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void
WrapLabel::on_size_allocate(Gtk::Allocation &alloc) // IN: Our allocation
{
   Gtk::Label::on_size_allocate(alloc);

   SetWrapWidth(alloc.get_width());
}


/*
 *-----------------------------------------------------------------------------
 *
 * view::WrapLabel::SetWrapWidth --
 *
 *      Sets the point at which the text should wrap.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

void
WrapLabel::SetWrapWidth(int width) // IN: The wrap width
{
   if (width == 0) {
      return;
   }

   int xPadding, yPadding;
   get_padding(xPadding, yPadding);

   width -= 2 * xPadding;

   /*
    * We may need to reset the wrap width, so do this regardless of whether
    * or not we've changed the width.
    */
   get_layout()->set_width(width * Pango::SCALE);

   int unused;
   get_layout()->get_pixel_size(unused, mWrapHeight);
   mWrapHeight += 2 * yPadding;

   if (mWrapWidth != width) {
      mWrapWidth = width;
      queue_resize();
   }
}


}; /* namespace view */

#endif // GTKMM_MAJOR_VERSION < 3
