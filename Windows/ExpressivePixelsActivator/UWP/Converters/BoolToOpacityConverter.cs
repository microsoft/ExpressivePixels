using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    /// <summary>
    /// Value converter that translates true to <see cref="Visibility.Visible"/> and false to
    /// <see cref="Visibility.Collapsed"/>.
    /// </summary>
    public sealed class BooleanToOpacityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            string[] opacityPair = ((string)parameter).Split(',');
            return (bool)value ? System.Convert.ToDouble(opacityPair[0]) : System.Convert.ToDouble(opacityPair[1]);
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }
    }
}

