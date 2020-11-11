using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    public class EmptyStringToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            string emptyString = value as string;

            bool inverse = (parameter != null && String.Compare((string)parameter, "True", StringComparison.OrdinalIgnoreCase) == 0);
            return (emptyString != null && emptyString.Length > 0) ? (inverse ? Visibility.Collapsed : Visibility.Visible) : (inverse ? Visibility.Visible : Visibility.Collapsed);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            throw new NotImplementedException();
        }
    }
}
