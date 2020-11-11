using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    /// <summary>
    /// Value converter that translates true to <see cref="Visibility.Visible"/> and false to
    /// <see cref="Visibility.Collapsed"/>.
    /// </summary>
    public sealed class BooleanToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            bool inverse = (parameter != null && String.Compare((string) parameter, "True", StringComparison.OrdinalIgnoreCase) == 0);            
            return (value is bool && (bool)value) ? (inverse ? Visibility.Collapsed : Visibility.Visible) : (inverse ? Visibility.Visible : Visibility.Collapsed);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            return value is Visibility && (Visibility)value == Visibility.Visible;
        }
    }
}
