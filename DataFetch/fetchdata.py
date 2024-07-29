import yfinance as yf
import pandas as pd
from datetime import datetime, timedelta


dow_30_symbols = [
    'AAPL', 'AMGN', 'AXP', 'BA', 'CAT', 'CRM', 'CSCO', 'CVX', 'DIS', 'DOW',
    'GS', 'HD', 'HON', 'IBM', 'INTC', 'JNJ', 'JPM', 'KO', 'MCD', 'MMM',
    'MRK', 'MSFT', 'NKE', 'PG', 'TRV', 'UNH', 'V', 'VZ', 'WBA', 'WMT'
]

end_date = datetime.today()
start_date = end_date - timedelta(days=5*365)

all_data = pd.DataFrame()

for symbol in dow_30_symbols:
    stock_data = yf.download(symbol, start=start_date, end=end_date)
    stock_data.reset_index(inplace = True)
    stock_data.to_csv('Data/' + symbol + '_data.csv')


