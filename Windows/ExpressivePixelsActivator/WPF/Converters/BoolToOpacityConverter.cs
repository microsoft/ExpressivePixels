using System;
using System.Globalization;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    class BoolToOpacityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            string[] opacityPair = ((string)parameter).Split(',');
            return (bool)value ? System.Convert.ToDouble(opacityPair[0]) : System.Convert.ToDouble(opacityPair[1]);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            throw new NotImplementedException();
        }
    }
}
