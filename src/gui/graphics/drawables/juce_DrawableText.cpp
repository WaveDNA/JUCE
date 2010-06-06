/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_DrawableText.h"
#include "juce_DrawableComposite.h"


//==============================================================================
DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : text (other.text),
      font (other.font),
      colour (other.colour),
      justification (other.justification),
      bounds (other.bounds),
      fontSizeControlPoint (other.fontSizeControlPoint)
{
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    text = newText;
}

void DrawableText::setColour (const Colour& newColour)
{
    colour = newColour;
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    font = newFont;

    if (applySizeAndScale)
    {
        Point<float> corners[3];
        bounds.resolveThreePoints (corners, parent);

        setFontSizeControlPoint (RelativePoint (bounds.getPointForInternalCoord (corners,
                                    Point<float> (font.getHorizontalScale() * font.getHeight(), font.getHeight()))));
    }
}

void DrawableText::setJustification (const Justification& newJustification)
{
    justification = newJustification;
}

void DrawableText::setBoundingBox (const RelativeParallelogram& newBounds)
{
    bounds = newBounds;
}

void DrawableText::setFontSizeControlPoint (const RelativePoint& newPoint)
{
    fontSizeControlPoint = newPoint;
}

//==============================================================================
void DrawableText::render (const Drawable::RenderingContext& context) const
{
    Point<float> points[3];
    bounds.resolveThreePoints (points, parent);

    const float w = Line<float> (points[0], points[1]).getLength();
    const float h = Line<float> (points[0], points[2]).getLength();

    const Point<float> fontCoords (bounds.getInternalCoordForPoint (points, fontSizeControlPoint.resolve (parent)));
    const float fontHeight = jlimit (1.0f, h, fontCoords.getY());
    const float fontWidth = jlimit (0.01f, w, fontCoords.getX());

    Font f (font);
    f.setHeight (fontHeight);
    f.setHorizontalScale (fontWidth / fontHeight);

    context.g.setColour (colour.withMultipliedAlpha (context.opacity));

    GlyphArrangement ga;
    ga.addFittedText (f, text, 0, 0, w, h, justification, 0x100000);
    ga.draw (context.g,
             AffineTransform::fromTargetPoints (0, 0, points[0].getX(), points[0].getY(),
                                                w, 0, points[1].getX(), points[1].getY(),
                                                0, h, points[2].getX(), points[2].getY())
                .followedBy (context.transform));
}

const Rectangle<float> DrawableText::getBounds() const
{
    return bounds.getBounds (parent);
}

bool DrawableText::hitTest (float x, float y) const
{
    Path p;
    bounds.getPath (p, parent);
    return p.contains (x, y);
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
}

void DrawableText::invalidatePoints()
{
}

//==============================================================================
const Identifier DrawableText::valueTreeType ("Text");

const Identifier DrawableText::ValueTreeWrapper::text ("text");
const Identifier DrawableText::ValueTreeWrapper::colour ("colour");
const Identifier DrawableText::ValueTreeWrapper::font ("font");
const Identifier DrawableText::ValueTreeWrapper::justification ("justification");
const Identifier DrawableText::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableText::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableText::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableText::ValueTreeWrapper::fontSizeAnchor ("fontSizeAnchor");

//==============================================================================
DrawableText::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

const String DrawableText::ValueTreeWrapper::getText() const
{
    return state [text].toString();
}

void DrawableText::ValueTreeWrapper::setText (const String& newText, UndoManager* undoManager)
{
    state.setProperty (text, newText, undoManager);
}

Value DrawableText::ValueTreeWrapper::getTextValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (text, undoManager);
}

const Colour DrawableText::ValueTreeWrapper::getColour() const
{
    return Colour::fromString (state [colour].toString());
}

void DrawableText::ValueTreeWrapper::setColour (const Colour& newColour, UndoManager* undoManager)
{
    state.setProperty (colour, newColour.toString(), undoManager);
}

const Justification DrawableText::ValueTreeWrapper::getJustification() const
{
    return Justification ((int) state [justification]);
}

void DrawableText::ValueTreeWrapper::setJustification (const Justification& newJustification, UndoManager* undoManager)
{
    state.setProperty (justification, newJustification.getFlags(), undoManager);
}

const Font DrawableText::ValueTreeWrapper::getFont() const
{
    return Font::fromString (state [font]);
}

void DrawableText::ValueTreeWrapper::setFont (const Font& newFont, UndoManager* undoManager)
{
    state.setProperty (font, newFont.toString(), undoManager);
}

Value DrawableText::ValueTreeWrapper::getFontValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (font, undoManager);
}

const RelativeParallelogram DrawableText::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state [topLeft].toString(), state [topRight].toString(), state [bottomLeft].toString());
}

void DrawableText::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

const RelativePoint DrawableText::ValueTreeWrapper::getFontSizeAndScaleAnchor() const
{
    return state [fontSizeAnchor].toString();
}

void DrawableText::ValueTreeWrapper::setFontSizeAndScaleAnchor (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (fontSizeAnchor, p.toString(), undoManager);
}

const Rectangle<float> DrawableText::refreshFromValueTree (const ValueTree& tree, ImageProvider*)
{
    ValueTreeWrapper v (tree);
    setName (v.getID());

    const RelativeParallelogram newBounds (v.getBoundingBox());
    const RelativePoint newFontPoint (v.getFontSizeAndScaleAnchor());
    const Colour newColour (v.getColour());
    const Justification newJustification (v.getJustification());
    const String newText (v.getText());
    const Font newFont (v.getFont());

    if (text != newText || font != newFont || justification != newJustification
         || colour != newColour || bounds != newBounds || newFontPoint != fontSizeControlPoint)
    {
        const Rectangle<float> damage (getBounds());

        setBoundingBox (newBounds);
        setFontSizeControlPoint (newFontPoint);
        setColour (newColour);
        setFont (newFont, false);
        setJustification (newJustification);
        setText (newText);

        return damage.getUnion (getBounds());

    }

    return Rectangle<float>();
}

const ValueTree DrawableText::createValueTree (ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    v.setText (text, 0);
    v.setFont (font, 0);
    v.setJustification (justification, 0);
    v.setColour (colour, 0);
    v.setBoundingBox (bounds, 0);
    v.setFontSizeAndScaleAnchor (fontSizeControlPoint, 0);

    return tree;
}


END_JUCE_NAMESPACE
