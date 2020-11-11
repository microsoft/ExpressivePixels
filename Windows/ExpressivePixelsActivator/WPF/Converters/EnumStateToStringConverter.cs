using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    public class EnumStateToStringConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo language)
        {
            Dictionary<string, string> setDictionary = new Dictionary<string, string>();
            var sets = ((string)parameter).Split(';');
            foreach(string set in sets)
            {
                var pair = set.Split(',');
                setDictionary[pair[0]] = pair[1];
            }
            return setDictionary[value.ToString()];
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo language)
        {
            throw new NotImplementedException();
        }
    }
}
