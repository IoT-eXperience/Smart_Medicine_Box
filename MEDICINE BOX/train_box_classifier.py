import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report
import joblib

# Load the data
# Assumes box_tilt_log.csv has columns: Angle_X_deg, Angle_Y_deg, Box_Status
# Box_Status should be 'Box Closed' or 'Box Open'
data = pd.read_csv('box_tilt_log.csv')

# Features and label
X = data[['Angle_X_deg', 'Angle_Y_deg']]
y = data['Box_Status'].map({'Box Closed': 0, 'Box Open': 1})  # Encode labels

# Split into train/test
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train classifier
clf = LogisticRegression()
clf.fit(X_train, y_train)

# Evaluate
y_pred = clf.predict(X_test)
print(classification_report(y_test, y_pred, target_names=['Box Closed', 'Box Open']))

# Save the model
joblib.dump(clf, 'box_classifier.joblib')
print("Model saved as box_classifier.joblib") 