using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    public sealed class DoubleToIntConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            double dval = (double)value;
            return (int)dval;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            return new NotImplementedException();
        }
    }
}
