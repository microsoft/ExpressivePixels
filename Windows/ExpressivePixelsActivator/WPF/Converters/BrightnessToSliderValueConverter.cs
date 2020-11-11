using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    public sealed class BrightnessToSliderValueConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            if (parameter == null)
                throw new Exception("BrightnessToSliderValueConverter exception Range parameter missing");
            double dparameter = System.Convert.ToDouble((string) parameter);
            return  (double)value * dparameter;             
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            if (parameter == null)
                throw new Exception("BrightnessToSliderValueConverter exception Range parameter missing");
            double dparameter = System.Convert.ToDouble((string)parameter);
            return ((double)value / dparameter);
        }
    }
}
