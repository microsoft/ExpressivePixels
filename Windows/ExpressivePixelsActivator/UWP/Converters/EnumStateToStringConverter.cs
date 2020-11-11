using System;
using System.Collections.Generic;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace Microsoft.ExpressivePixels.Converters
{
    /// <summary>
    /// Value converter that translates true to <see cref="Visibility.Visible"/> and false to
    /// <see cref="Visibility.Collapsed"/>.
    /// </summary>
    public sealed class EnumStateToStringConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            Dictionary<string, string> setDictionary = new Dictionary<string, string>();
            var sets = ((string)parameter).Split(';');
            foreach (string set in sets)
            {
                var pair = set.Split(',');
                setDictionary[pair[0]] = pair[1];
            }
            return setDictionary[value.ToString()];
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }
    }
}

