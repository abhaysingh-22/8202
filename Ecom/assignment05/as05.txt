document.addEventListener('DOMContentLoaded', () => {
    const cartItemsContainer = document.getElementById('cart-items');
    const cartTotalElement = document.getElementById('cart-total');
    const cartCountElement = document.getElementById('cart-count');
    const checkoutBtn = document.getElementById('checkout-btn');

    // Retrieve cart from session storage or initialize empty object
    let cart = JSON.parse(sessionStorage.getItem('shoppingCart')) || {};

    const renderCart = () => {
        cartItemsContainer.innerHTML = '';
        let total = 0;
        let itemCount = 0;

        const cartKeys = Object.keys(cart);
        if (cartKeys.length === 0) {
            cartItemsContainer.innerHTML = '<p class="empty-msg">Cart is empty</p>';
            cartTotalElement.textContent = '$0.00';
            cartCountElement.textContent = '0 items';
            return;
        }

        cartKeys.forEach(id => {
            const item = cart[id];
            const itemTotal = item.price * item.quantity;
            total += itemTotal;
            itemCount += item.quantity;

            const itemDiv = document.createElement('div');
            itemDiv.className = 'cart-item';
            itemDiv.innerHTML = `
                <div class="cart-item-info">
                    <span class="cart-item-title">${item.name}</span>
                    <span class="cart-item-price">$${item.price.toFixed(2)} x ${item.quantity}</span>
                </div>
                <div class="cart-item-qty">$${itemTotal.toFixed(2)}</div>
            `;
            cartItemsContainer.appendChild(itemDiv);
        });

        cartTotalElement.textContent = `$${total.toFixed(2)}`;
        cartCountElement.textContent = `${itemCount} items`;
    };

    const addToCart = (id, name, price) => {
        if (cart[id]) {
            cart[id].quantity += 1;
        } else {
            cart[id] = { name, price, quantity: 1 };
        }
        // Save back to session storage
        sessionStorage.setItem('shoppingCart', JSON.stringify(cart));
        renderCart();
    };

    document.querySelectorAll('.add-btn').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const id = e.target.getAttribute('data-id');
            const name = e.target.getAttribute('data-name');
            const price = parseFloat(e.target.getAttribute('data-price'));
            addToCart(id, name, price);
        });
    });

    checkoutBtn.addEventListener('click', () => {
        if (Object.keys(cart).length === 0) {
            alert('Your cart is empty!');
            return;
        }
        
        alert('Checkout successful! Session data will be cleared.');
        
        // Clear session on successful checkout
        cart = {};
        sessionStorage.removeItem('shoppingCart');
        renderCart();
    });

    // Render cart on page load
    renderCart();
});
