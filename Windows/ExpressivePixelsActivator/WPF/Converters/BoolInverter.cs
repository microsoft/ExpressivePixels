using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Markup;

namespace Microsoft.ExpressivePixels.Converters
{
    public class BoolInverter : MarkupExtension, IValueConverter
    {
        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            return (!(bool)value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            return Convert(value, targetType, parameter, language);
        }
    }
}
