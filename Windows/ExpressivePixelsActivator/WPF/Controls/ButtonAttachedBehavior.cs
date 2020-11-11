using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace Microsoft.ExpressivePixels.Controls
{
    public static class ButtonAttachedBehavior
    {
        public static readonly DependencyProperty LabelProperty = DependencyProperty.RegisterAttached("Label", typeof(string), typeof(ButtonAttachedBehavior), new PropertyMetadata(null));

        public static string GetLabel(DependencyObject d)
        {
            return (string)d.GetValue(LabelProperty);
        }

        public static void SetLabel(DependencyObject d, string value)
        {
            d.SetValue(LabelProperty, value);
        }

        public static readonly DependencyProperty GlyphProperty = DependencyProperty.RegisterAttached("Glyph", typeof(string), typeof(ButtonAttachedBehavior), new PropertyMetadata(null));

        public static string GetGlyph(DependencyObject d)
        {
            return (string)d.GetValue(GlyphProperty);
        }

        public static void SetGlyph(DependencyObject d, string value)
        {
            d.SetValue(GlyphProperty, value);
        }


        public static readonly DependencyProperty GlyphContentProperty = DependencyProperty.RegisterAttached("GlyphContent", typeof(ControlTemplate), typeof(ButtonAttachedBehavior), new PropertyMetadata(null));

        public static ControlTemplate GetGlyphContent(DependencyObject d)
        {
            return (ControlTemplate)d.GetValue(GlyphContentProperty);
        }

        public static void SetGlyphContent(DependencyObject d, string value)
        {
            d.SetValue(GlyphContentProperty, value);
        }



        public static readonly DependencyProperty GlyphContentHeightProperty = DependencyProperty.RegisterAttached("GlyphContentHeight", typeof(double), typeof(ButtonAttachedBehavior), new PropertyMetadata(10.0));

        public static double GetGlyphContentHeight(DependencyObject d)
        {
            return (double)d.GetValue(GlyphContentHeightProperty);
        }

        public static void SetGlyphContentHeight(DependencyObject d, string value)
        {
            d.SetValue(GlyphContentHeightProperty, value);
        }



        public static readonly DependencyProperty GlyphForegroundProperty = DependencyProperty.RegisterAttached("GlyphForeground", typeof(SolidColorBrush), typeof(ButtonAttachedBehavior), new PropertyMetadata(new SolidColorBrush(Colors.White)));

        public static SolidColorBrush GetGlyphForeground(DependencyObject d)
        {
            return (SolidColorBrush)d.GetValue(GlyphForegroundProperty);
        }

        public static void SetGlyphForeground(DependencyObject d, string value)
        {
            d.SetValue(GlyphForegroundProperty, value);
        }


        public static readonly DependencyProperty SelectionToggleProperty = DependencyProperty.RegisterAttached("SelectionToggle", typeof(bool), typeof(ButtonAttachedBehavior), new PropertyMetadata(false));

        public static bool GetSelectionToggle(DependencyObject d)
        {
            return (bool)d.GetValue(SelectionToggleProperty);
        }

        public static void SetSelectionToggle(DependencyObject d, string value)
        {
            d.SetValue(SelectionToggleProperty, value);
        }


        public static readonly DependencyProperty RoundedRadiusProperty = DependencyProperty.RegisterAttached("RoundedRadius", typeof(double), typeof(ButtonAttachedBehavior), new PropertyMetadata(0.0));

        public static double GetRoundedRadius(DependencyObject d)
        {
            return (double)d.GetValue(RoundedRadiusProperty);
        }

        public static void SetRoundedRadius(DependencyObject d, string value)
        {
            d.SetValue(RoundedRadiusProperty, value);
        }



        public static readonly DependencyProperty LabelMarginProperty = DependencyProperty.RegisterAttached("LabelMargin", typeof(Thickness), typeof(ButtonAttachedBehavior), new PropertyMetadata(new Thickness()));

        public static Thickness GetLabelMargin(DependencyObject d)
        {
            return (Thickness)d.GetValue(LabelMarginProperty);
        }

        public static void SetLabelMargin(DependencyObject d, string value)
        {
            d.SetValue(LabelMarginProperty, value);
        }


    }
}
