using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Media;

namespace Microsoft.ExpressivePixels.Converters
{
    public sealed class TrueFalseBrushConverter : IValueConverter
    {
        public Brush TrueBrush { get; set; }
        public Brush FalseBrush { get; set; }


        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            return (value is bool && (bool)value) ? TrueBrush : FalseBrush;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            throw new Exception();
        }
    }
}
